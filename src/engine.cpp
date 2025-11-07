#include "engine.hpp"
#include <algorithm>
#include <limits>
#include <cmath>
#include <chrono>
#include <random>

using namespace std;
using namespace std::chrono;

// Profiling counters
static long long ttLookupTime = 0;
static long long evalTime = 0;
static long long moveGenTime = 0;
static int ttLookupCalls = 0;
static int evalCalls = 0;
static int moveGenCalls = 0;

// RNG for root move randomization (opening variety)
static std::mt19937 engineRng((uint32_t)std::chrono::steady_clock::now().time_since_epoch().count());

// Get the best move for the current position
Move Engine::getBestMove(ChessGame& game, int depth) {
    nodesSearched = 0;  // Reset counter at start of search
    ttHits = 0;  // Reset TT hits counter

    
    vector<Move> legalMoves = game.getLegalMoves();
    
    if (legalMoves.empty()) {
        return Move(-1, -1, -1, -1); // No legal moves
    }
    
    // Validate that moves don't leave king in check
    vector<Move> validatedMoves;
    for (const Move& move : legalMoves) {
        game.makeMoveForEngine(move);
        bool leavesKingInCheck = game.isInCheck(); // Check if OUR king is in check after move
        game.undoMove();
        
        if (!leavesKingInCheck) {
            validatedMoves.push_back(move);
        }
    }
    
    if (validatedMoves.empty()) {
        return Move(-1, -1, -1, -1); // No valid moves
    }
    // Shuffle validated moves at root to vary opening choices between games
    // TEMPORARILY DISABLED for testing - shuffle randomizes even with good eval!
    // std::shuffle(validatedMoves.begin(), validatedMoves.end(), engineRng);
    // Order validated moves at root using MVV-LVA so captures/promotions are searched early.
    // This is a cheap sort on the root move list (normally ~20-40 moves) and actually
    // helps pruning—it's negligible compared to the cost of search and usually speeds it up.
    // Use a root-specific ordering which promotes checks/mates above captures
    // Initial root ordering so the first iteration searches checks/mates early
    // First, try a small depth-limited mate prover to quickly detect forced mates
    Move mateMove(-1,-1,-1,-1);
    int mateProverDepth = std::min(depth, 8);
    if (mateProverDepth > 0 && rootMateProver(game, mateProverDepth, mateMove)) {
        return mateMove; // Found a forced mate at root; return immediately
    }

    // Otherwise, perform normal root ordering
    orderRootMoves(game, validatedMoves);
    
    bool isWhiteTurn = game.isWhiteToMove();
    Move bestMove = validatedMoves[0];
    
    // ITERATIVE DEEPENING: Search from depth 1 to target depth
    for (int currentDepth = 1; currentDepth <= depth; currentDepth++) {
        
        // Move pvMove to front of list if it's valid (for better move ordering)
        if (pvMove.startRow != -1) {
            auto it = find_if(validatedMoves.begin(), validatedMoves.end(), 
                [this](const Move& m) {
                    return m.startRow == pvMove.startRow && 
                           m.startColumn == pvMove.startColumn &&
                           m.targetRow == pvMove.targetRow && 
                           m.targetColumn == pvMove.targetColumn;
                });
            if (it != validatedMoves.end()) {
                // Swap pvMove to front
                Move temp = *it;
                validatedMoves.erase(it);
                validatedMoves.insert(validatedMoves.begin(), temp);
            }
        }
            // Re-apply root ordering to the remaining moves (keep PV pinned at index 0)
            if (validatedMoves.size() > 1) {
                // Order only the moves after the PV to preserve PV stability
                vector<Move> remainder(validatedMoves.begin() + 1, validatedMoves.end());
                orderRootMoves(game, remainder);
                // copy reordered remainder back into validatedMoves
                for (size_t i = 0; i < remainder.size(); ++i) {
                    validatedMoves[i + 1] = remainder[i];
                }
            }
        
        if (isWhiteTurn) {
            // White wants to maximize evaluation
            double bestEval = -numeric_limits<double>::infinity();
            
            for (const Move& move : validatedMoves) {
                game.makeMoveForEngine(move);
                
                double eval = alphabeta(game, currentDepth - 1, -numeric_limits<double>::infinity(), 
                                       numeric_limits<double>::infinity(), false, true, 1);
                
                game.undoMove();

                if (eval > bestEval) {
                    bestEval = eval;
                    bestMove = move;
                }
            }

        } else {
            // Black wants to minimize evaluation
            double bestEval = numeric_limits<double>::infinity();
            
            for (const Move& move : validatedMoves) {
                game.makeMoveForEngine(move);
                
                double eval = alphabeta(game, currentDepth - 1, -numeric_limits<double>::infinity(), 
                                       numeric_limits<double>::infinity(), true, true, 1);
                
                game.undoMove();

                if (eval < bestEval) {
                    bestEval = eval;
                    bestMove = move;
                }
            }
        }
        
        // Update pvMove for next iteration
        pvMove = bestMove;
    }
    
    // Clear undo stack after search is complete
    game.clearUndoStack();
    
    // Print profiling results (commented out for cleaner output)
    // cout << "\n=== PROFILING RESULTS ===" << endl;
    // cout << "TT Lookups: " << ttLookupCalls << " calls, " 
    //      << (ttLookupTime / 1000.0) << " ms (" 
    //      << (ttLookupTime / (double)ttLookupCalls / 1000.0) << " ms avg)" << endl;
    // cout << "Evaluations: " << evalCalls << " calls, " 
    //      << (evalTime / 1000.0) << " ms (" 
    //      << (evalTime / (double)evalCalls / 1000.0) << " ms avg)" << endl;
    // cout << "Move Generation: " << moveGenCalls << " calls, " 
    //      << (moveGenTime / 1000.0) << " ms (" 
    //      << (moveGenTime / (double)moveGenCalls / 1000.0) << " ms avg)" << endl;
    // cout << "Total Profiled: " 
    //      << ((ttLookupTime + evalTime + moveGenTime) / 1000.0) << " ms" << endl;
    // cout << "TT Table Size: " << transpositionTable.size() << " unique positions" << endl;
    // cout << "=========================" << endl;
    
    return bestMove;
}

// Fast move ordering using MVV-LVA (Most Valuable Victim - Least Valuable Attacker)
// No make/undo moves - just looks at the board state
void Engine::fastOrderMoves(vector<Move>& moves) {
    struct MoveScore { Move move; int score; };
    vector<MoveScore> scoredMoves;
    
    for (const Move& move : moves) {
        int score = 0;
        
        // Detect capture by inspecting the board square at the target
        int capturedPiece = EMPTY;
        if (move.moveType == EN_PASSANT) {
            capturedPiece = board[move.startRow][move.targetColumn];
        } else {
            capturedPiece = board[move.targetRow][move.targetColumn];
        }
        
        int movingPiece = board[move.startRow][move.startColumn];
        
        if (!isEmpty(capturedPiece)) {
            // MVV-LVA: (Victim value * 10) - Attacker value
            // This prioritizes capturing valuable pieces with less valuable pieces
            int victimValue = 0;
            int attackerValue = 0;
            
            int victimType = capturedPiece & 0b0111;
            int attackerType = movingPiece & 0b0111;
            
            switch(victimType) {
                case 0b0001: victimValue = 1; break;  // pawn
                case 0b0011: victimValue = 3; break;  // knight
                case 0b0100: victimValue = 3; break;  // bishop
                case 0b0010: victimValue = 5; break;  // rook
                case 0b0101: victimValue = 9; break;  // queen
                default: victimValue = 0; break;
            }
            
            switch(attackerType) {
                case 0b0001: attackerValue = 1; break;  // pawn
                case 0b0011: attackerValue = 3; break;  // knight
                case 0b0100: attackerValue = 3; break;  // bishop
                case 0b0010: attackerValue = 5; break;  // rook
                case 0b0101: attackerValue = 9; break;  // queen
                case 0b0110: attackerValue = 10; break; // king (discourage king captures)
                default: attackerValue = 0; break;
            }
            
            // Captures: higher score for better MVV-LVA
            score = 1000 + (victimValue * 10) - attackerValue;
        }
        // Promotions are also valuable
        else if (move.moveType == PAWN_PROMOTION) {
            score = 900;  // High priority
        }
        // Quiet moves get low priority
        else {
            score = 0;
        }
        
        scoredMoves.push_back({move, score});
    }
    
    // Sort descending by score
    sort(scoredMoves.begin(), scoredMoves.end(), [](const MoveScore& a, const MoveScore& b) {
        return a.score > b.score;
    });
    
    // Copy back
    moves.clear();
    for (const auto& ms : scoredMoves) {
        moves.push_back(ms.move);
    }
}

// Generate only capture moves for quiescence search
vector<Move> Engine::generateCaptureMoves(ChessGame& game) {
    vector<Move> allMoves = game.getLegalMoves();
    vector<Move> captures;
    
    for (const Move& move : allMoves) {
        // Check if it's a capture
        int capturedPiece = EMPTY;
        if (move.moveType == EN_PASSANT) {
            capturedPiece = board[move.startRow][move.targetColumn];
        } else {
            capturedPiece = board[move.targetRow][move.targetColumn];
        }
        
        // Include captures and promotions (promotions are also tactical)
        if (!isEmpty(capturedPiece) || move.moveType == PAWN_PROMOTION) {
            captures.push_back(move);
        }
    }
    
    return captures;
}

// Quiescence search - search tactical moves until position is quiet
double Engine::quiescence(ChessGame& game, double alpha, double beta, bool isMaximizing, int qDepth) {
    nodesSearched++;  // Count this node
    
    // Limit quiescence depth to prevent explosion (more aggressive limit)
    const int MAX_QUIESCENCE_DEPTH = 6;
    if (qDepth >= MAX_QUIESCENCE_DEPTH) {
        auto evalStart = high_resolution_clock::now();
        double result = evaluator.evaluate(game);
        auto evalEnd = high_resolution_clock::now();
        evalTime += duration_cast<microseconds>(evalEnd - evalStart).count();
        evalCalls++;
        return result;
    }
    
    // Stand pat score - the evaluation if we don't make any more captures
    auto evalStart = high_resolution_clock::now();
    double standPat = evaluator.evaluate(game);
    auto evalEnd = high_resolution_clock::now();
    evalTime += duration_cast<microseconds>(evalEnd - evalStart).count();
    evalCalls++;
    
    if (isMaximizing) {
        // Can we already improve alpha without searching?
        if (standPat >= beta) {
            return beta;  // Beta cutoff
        }
        if (standPat > alpha) {
            alpha = standPat;  // Improve alpha
        }
    } else {
        // Can we already improve beta without searching?
        if (standPat <= alpha) {
            return alpha;  // Alpha cutoff
        }
        if (standPat < beta) {
            beta = standPat;  // Improve beta
        }
    }
    
    // Generate and search only capture moves
    vector<Move> captureMoves = generateCaptureMoves(game);
    
    // If no captures, position is quiet - return stand pat
    if (captureMoves.empty()) {
        return standPat;
    }
    
    // Order captures by MVV-LVA
    fastOrderMoves(captureMoves);
    
    // Delta pruning threshold - biggest possible material gain (queen = 9)
    const double BIG_DELTA = 9.0 + 1.0;  // Queen value + safety margin
    
    if (isMaximizing) {
        double maxEval = standPat;
        
        // Delta pruning - if even capturing a queen can't improve alpha, skip search
        if (standPat + BIG_DELTA < alpha) {
            return alpha;
        }
        
        for (const Move& move : captureMoves) {
            game.makeMoveForEngine(move);
            double eval = quiescence(game, alpha, beta, false, qDepth + 1);
            game.undoMove();
            
            maxEval = max(maxEval, eval);
            alpha = max(alpha, eval);
            if (beta <= alpha) {
                break;  // Beta cutoff
            }
        }
        return maxEval;
        
    } else {
        double minEval = standPat;
        
        // Delta pruning - if even capturing a queen can't improve beta, skip search
        if (standPat - BIG_DELTA > beta) {
            return beta;
        }
        
        for (const Move& move : captureMoves) {
            game.makeMoveForEngine(move);
            double eval = quiescence(game, alpha, beta, true, qDepth + 1);
            game.undoMove();
            
            minEval = min(minEval, eval);
            beta = min(beta, eval);
            if (beta <= alpha) {
                break;  // Alpha cutoff
            }
        }
        return minEval;
    }
}

// Alpha-beta pruning (optimized minimax)
double Engine::alphabeta(ChessGame& game, int depth, double alpha, double beta, bool isMaximizing, bool allowNullMove, int ply) {
    nodesSearched++;  // Count this node
    
    // Check transposition table BEFORE generating moves (expensive operation)
    auto ttStart = high_resolution_clock::now();
    uint64_t posKey = game.getZobristHash();
    auto it = transpositionTable.find(posKey);
    auto ttEnd = high_resolution_clock::now();
    ttLookupTime += duration_cast<microseconds>(ttEnd - ttStart).count();
    ttLookupCalls++;
    
    // Use TT entry if it was searched at equal or greater depth
    // (A position searched deeper is more accurate)
    if (it != transpositionTable.end() && it->second.depth >= depth) {
        ttHits++;
        double stored = it->second.score;
        // Ignore entries with non-finite scores (defensive)
        if (!std::isfinite(stored)) {
            // fall through and search normally
        } else {
            const double MATE_SCORE = 100000.0;
            // If the entry encodes a mate-in-N from that position, reconstruct
            // the score for the current 'ply' so mate-distance is preserved.
            if (it->second.mateDistance > 0 && std::abs(stored) >= (MATE_SCORE - 1000.0)) {
                int D = it->second.mateDistance; // plies from stored position to mate
                // Reconstruct a score relative to current ply: score = sign * (MATE_SCORE - (ply + D))
                double sign = (stored > 0.0) ? 1.0 : -1.0;
                double adjusted = sign * (MATE_SCORE - (ply + D));

                if (it->second.bound == TTBound::EXACT) {
                    return adjusted;
                } else if (it->second.bound == TTBound::LOWER) {
                    if (adjusted > alpha) alpha = adjusted;
                    if (alpha >= beta) return adjusted;
                } else if (it->second.bound == TTBound::UPPER) {
                    if (adjusted < beta) beta = adjusted;
                    if (alpha >= beta) return adjusted;
                }
            } else {
                // Non-mate entries: use previous bound-aware logic
                if (it->second.bound == TTBound::EXACT) {
                    return stored;
                } else if (it->second.bound == TTBound::LOWER) {
                    // Stored score is a lower bound: true score >= stored
                    if (stored > alpha) alpha = stored;
                    if (alpha >= beta) return stored;
                } else if (it->second.bound == TTBound::UPPER) {
                    // Stored score is an upper bound: true score <= stored
                    if (stored < beta) beta = stored;
                    if (alpha >= beta) return stored;
                }
            }
        }
    }
    
    if(depth == 0){
        // Instead of static eval, call quiescence search to resolve captures
        return quiescence(game, alpha, beta, isMaximizing);
    }
    
    // NULL MOVE PRUNING
    // Try giving opponent a free move - if we're still winning, cutoff early
    const int NULL_MOVE_REDUCTION = 3;  // Search 3 plies less
    
    // Guard null-move when beta is infinite: applying null-move with an infinite
    // window leads to (-beta, -beta+1) = (-inf, -inf) which produces sentinel
    // values that can cascade. Only attempt null-move when beta is finite.
    // Do not attempt null-move pruning if TT indicates a mate is nearby or other
    // unsafe conditions. Null-move can irreversibly prune mate lines.
    bool ttIndicatesMate = (it != transpositionTable.end() && it->second.mateDistance > 0);
    if (allowNullMove && depth >= NULL_MOVE_REDUCTION + 1 && !game.isInCheck() &&
        std::isfinite(beta) && !ttIndicatesMate) {
        // Make null move
        game.makeNullMove();
        
        // Search with reduced depth and flipped window
                double nullScore = -alphabeta(game, depth - 1 - NULL_MOVE_REDUCTION, -beta, -beta + 1, !isMaximizing, false, ply+1);
        
        // Undo null move
        game.undoNullMove();

        
        // If null move causes beta cutoff, we can prune
        if (nullScore >= beta) {
            return beta;  // Cutoff
        }
    }

    auto moveGenStart = high_resolution_clock::now();
    vector<Move> legalmoves = game.getLegalMoves();
    auto moveGenEnd = high_resolution_clock::now();
    moveGenTime += duration_cast<microseconds>(moveGenEnd - moveGenStart).count();
    moveGenCalls++;
    
    // If no legal moves, it's checkmate or stalemate
    if(legalmoves.empty()) {
        double eval;
        if(game.isInCheck()) {
            // Checkmate: return extreme values but prefer shorter mates
            const double MATE_SCORE = 100000.0;
            // 'ply' is the number of plies from the root to this node
            // If current side is checkmated (isMaximizing true), return a large negative
            // value increased by ply so that shorter mate (smaller ply) is worse.
            eval = isMaximizing ? (-MATE_SCORE + ply) : (MATE_SCORE - ply);
        } else {
            // Stalemate: penalize if we're winning, reward if we're losing
            // This makes the engine avoid stalemate when ahead and seek it when behind
            double materialScore = evaluator.materialCount(game);
            // If we're ahead (positive material), stalemate is BAD (-5000)
            // If we're behind (negative material), stalemate is GOOD (+5000)
            // The penalty/reward is proportional to material advantage
            eval = -materialScore * 500.0;  // Scale the penalty
        }
        // DON'T store terminal nodes in TT - they're position-specific
        // and should not be reused for other positions
        return eval;
    }
    
    // Simple move ordering: captures first (MVV-LVA), then quiet moves
    // Much faster than the complex orderMoves() which makes/undoes moves
    fastOrderMoves(legalmoves);
    
    if(isMaximizing){
        //white to move - maximise eval
        //initialise max Eval to -infinity (lowest possible eval)
        double maxEval = -numeric_limits<double>::infinity();
        double origAlpha = alpha;
        double origBeta = beta;

        //run through legal moves
        int moveCount = 0;
        for(const Move& move : legalmoves){
            game.makeMoveForEngine(move);
            
            double eval;
            
            // LATE MOVE REDUCTIONS (LMR)
            // Search first few moves at full depth, reduce depth for later moves
            const int FULL_DEPTH_MOVES = 4;  // First 4 moves at full depth
            const int REDUCTION = 2;          // Reduce by 2 plies
            
            // Determine if this move gives check/mate — if so, avoid LMR reductions
            bool givesCheck = game.isInCheck();
            bool givesMate = game.isInCheckmate();

            if (moveCount >= FULL_DEPTH_MOVES && depth >= 3 && !game.isInCheck() && !givesCheck && !givesMate) {
                // Search at reduced depth
                eval = alphabeta(game, depth - 1 - REDUCTION, alpha, beta, false, true, ply+1);

                // If reduced search beats alpha, re-search at full depth
                if (eval > alpha) {
                    eval = alphabeta(game, depth - 1, alpha, beta, false, true, ply+1);
                }
            } else {
                // Search first few moves or tactical positions at full depth
                eval = alphabeta(game, depth - 1, alpha, beta, false, true, ply+1);
            }
            
            game.undoMove();
            maxEval = max(maxEval, eval);
            alpha = max(alpha, eval);
            if (beta <= alpha) {
                break; // Beta cutoff
            }
            moveCount++;
        }
        
        // Store in transposition table (if finite) with proper bound
        if (std::isfinite(maxEval)) {
            TTBound bound;
            if (maxEval <= origAlpha) bound = TTBound::UPPER;
            else if (maxEval >= origBeta) bound = TTBound::LOWER;
            else bound = TTBound::EXACT;
            // If this is a mate score, compute mateDistance as plies from THIS position
            // to mate (D = plyAtMate - plyAtStore). Store it and mark as EXACT to avoid
            // losing mate info through bound semantics.
            int mateDist = 0;
            const double MATE_SCORE = 100000.0;
            if (std::abs(maxEval) >= (MATE_SCORE - 1000.0)) {
                int plyAtMate = static_cast<int>(std::round(MATE_SCORE - std::abs(maxEval)));
                int D = plyAtMate - ply; // plies from this position to mate
                if (D < 0) D = 0;
                mateDist = D;
                // Prefer storing mate entries as EXACT so they are returned precisely later
                bound = TTBound::EXACT;
            }
            transpositionTable[posKey] = {maxEval, depth, bound, mateDist};
        }
        return maxEval;
    } else {
    //black to move - minimise eval
    //initialise min Eval to +infinity (highest possible eval)
    double minEval = numeric_limits<double>::infinity();
    double origAlpha = alpha;
    double origBeta = beta;

        //run through legal moves
        int moveCount = 0;
        for(const Move& move : legalmoves){
            game.makeMoveForEngine(move);
            
            double eval;
            
            // LATE MOVE REDUCTIONS (LMR)
            const int FULL_DEPTH_MOVES = 4;
            const int REDUCTION = 2;
            
            // Determine if this move gives check/mate — if so, avoid LMR reductions
            bool givesCheck = game.isInCheck();
            bool givesMate = game.isInCheckmate();

            if (moveCount >= FULL_DEPTH_MOVES && depth >= 3 && !game.isInCheck() && !givesCheck && !givesMate) {
                // Search at reduced depth
                eval = alphabeta(game, depth - 1 - REDUCTION, alpha, beta, true, true, ply+1);

                // If reduced search beats beta, re-search at full depth
                if (eval < beta) {
                    eval = alphabeta(game, depth - 1, alpha, beta, true, true, ply+1);
                }
            } else {
                // Search first few moves or tactical positions at full depth
                eval = alphabeta(game, depth - 1, alpha, beta, true, true, ply+1);
            }
            
            game.undoMove();
            minEval = min(minEval, eval);
            beta = min(beta, eval);
            if (beta <= alpha) {
                break; // Alpha cutoff
            }
            moveCount++;
        }
        
        // Store in transposition table (if finite) with proper bound
        if (std::isfinite(minEval)) {
            TTBound bound;
            if (minEval <= origAlpha) bound = TTBound::UPPER;
            else if (minEval >= origBeta) bound = TTBound::LOWER;
            else bound = TTBound::EXACT;
            // If this is a mate score, compute mateDistance from this position to mate
            int mateDist = 0;
            const double MATE_SCORE = 100000.0;
            if (std::abs(minEval) >= (MATE_SCORE - 1000.0)) {
                int plyAtMate = static_cast<int>(std::round(MATE_SCORE - std::abs(minEval)));
                int D = plyAtMate - ply; // plies from this position to mate
                if (D < 0) D = 0;
                mateDist = D;
                bound = TTBound::EXACT;
            }
            transpositionTable[posKey] = {minEval, depth, bound, mateDist};
        }
        return minEval;
    }
}

// Set RNG seed used for root move randomization
void Engine::setRngSeed(uint64_t seed) {
    engineRng.seed((uint32_t)seed);
}

// Root-specific ordering: we can afford to make/unmake moves here to detect
// checks and checkmates and promote them above MVV-LVA captures so the
// search doesn't overlook forced mates.
void Engine::orderRootMoves(ChessGame& game, vector<Move>& moves) {
    struct MoveScore { Move move; int score; };
    vector<MoveScore> scored;

    for (const Move& move : moves) {
        int score = 0;

        // Make the move to detect checks/mates
        game.makeMoveForEngine(move);
        bool givesMate = game.isInCheckmate();
        game.undoMove();

        if (givesMate) {
            score += 20000; // huge bonus for mate
        }
        // Keep MVV-LVA capture scoring as a tiebreaker
        int capturedPiece = EMPTY;
        if (move.moveType == EN_PASSANT) {
            capturedPiece = board[move.startRow][move.targetColumn];
        } else {
            capturedPiece = board[move.targetRow][move.targetColumn];
        }
        int movingPiece = board[move.startRow][move.startColumn];
        if (!isEmpty(capturedPiece)) {
            int victimValue = 0;
            int attackerValue = 0;
            int victimType = capturedPiece & 0b0111;
            int attackerType = movingPiece & 0b0111;
            switch(victimType) {
                case 0b0001: victimValue = 1; break;
                case 0b0011: victimValue = 3; break;
                case 0b0100: victimValue = 3; break;
                case 0b0010: victimValue = 5; break;
                case 0b0101: victimValue = 9; break;
                default: victimValue = 0; break;
            }
            switch(attackerType) {
                case 0b0001: attackerValue = 1; break;
                case 0b0011: attackerValue = 3; break;
                case 0b0100: attackerValue = 3; break;
                case 0b0010: attackerValue = 5; break;
                case 0b0101: attackerValue = 9; break;
                case 0b0110: attackerValue = 10; break;
                default: attackerValue = 0; break;
            }
            score += 1000 + (victimValue * 10) - attackerValue;
        }

        // Promotions are also valuable
        if (move.moveType == PAWN_PROMOTION) score += 900;

        scored.push_back({move, score});
    }

    sort(scored.begin(), scored.end(), [](const MoveScore& a, const MoveScore& b) {
        return a.score > b.score;
    });

    moves.clear();
    for (const auto& ms : scored) moves.push_back(ms.move);
}

// Depth-limited proof search: attacker tries to force mate within depthLeft plies.
// Uses OR on attacker's nodes and AND on defender's nodes.
bool Engine::canForceMate(ChessGame& game, int depthLeft, bool attackerIsWhite) {
    // Terminal node: no legal moves
    vector<Move> legal = game.getLegalMoves();
    if (legal.empty()) {
        // If side to move is in check, it's checkmate for the side that just moved
        return game.isInCheck();
    }

    if (depthLeft <= 0) return false;

    bool sideToMoveIsAttacker = (game.isWhiteToMove() == attackerIsWhite);

    if (sideToMoveIsAttacker) {
        // Attacker needs at least one move that forces mate
        for (const Move& mv : legal) {
            // Heuristic: prefer checking/capturing moves first
            game.makeMoveForEngine(mv);
            bool res = canForceMate(game, depthLeft - 1, attackerIsWhite);
            game.undoMove();
            if (res) return true;
        }
        return false;
    } else {
        // Defender: all moves must still lead to mate
        for (const Move& mv : legal) {
            game.makeMoveForEngine(mv);
            bool res = canForceMate(game, depthLeft - 1, attackerIsWhite);
            game.undoMove();
            if (!res) return false; // defender found a way to avoid mate
        }
        return true; // all replies lead to mate
    }
}

bool Engine::rootMateProver(ChessGame& game, int maxDepth, Move& outMove) {
    bool attackerIsWhite = game.isWhiteToMove();
    vector<Move> legal = game.getLegalMoves();
    if (legal.empty()) return false;

    // Try increasing depths from 1..maxDepth
    for (int d = 1; d <= maxDepth; ++d) {
        for (const Move& mv : legal) {
            game.makeMoveForEngine(mv);
            // Only consider lines where attacker moves first, then defender replies
            bool forces = canForceMate(game, d - 1, attackerIsWhite);
            game.undoMove();
            if (forces) {
                outMove = mv;
                return true;
            }
        }
    }
    return false;
}