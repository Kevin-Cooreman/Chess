#include "engine.hpp"
#include <algorithm>
#include <limits>
#include <chrono>

using namespace std;
using namespace std::chrono;

// Profiling counters
static long long ttLookupTime = 0;
static long long evalTime = 0;
static long long moveGenTime = 0;
static int ttLookupCalls = 0;
static int evalCalls = 0;
static int moveGenCalls = 0;

// Get the best move for the current position
Move Engine::getBestMove(ChessGame& game, int depth) {
    nodesSearched = 0;  // Reset counter at start of search
    ttHits = 0;  // Reset TT hits counter
    // DON'T clear TT - reuse entries across searches and iterations!
    // transpositionTable.clear();  
    
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
        
        if (isWhiteTurn) {
            // White wants to maximize evaluation
            double bestEval = -numeric_limits<double>::infinity();
            
            for (const Move& move : validatedMoves) {
                game.makeMoveForEngine(move);
                
                double eval = alphabeta(game, currentDepth - 1, -numeric_limits<double>::infinity(), 
                                       numeric_limits<double>::infinity(), false);
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
                                       numeric_limits<double>::infinity(), true);
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
double Engine::alphabeta(ChessGame& game, int depth, double alpha, double beta, bool isMaximizing, bool allowNullMove) {
    nodesSearched++;  // Count this node
    
    // Check transposition table BEFORE generating moves (expensive operation)
    auto ttStart = high_resolution_clock::now();
    string posKey = game.getPositionKey();
    auto it = transpositionTable.find(posKey);
    auto ttEnd = high_resolution_clock::now();
    ttLookupTime += duration_cast<microseconds>(ttEnd - ttStart).count();
    ttLookupCalls++;
    
    // Use TT entry if it was searched at equal or greater depth
    // (A position searched deeper is more accurate)
    if (it != transpositionTable.end() && it->second.depth >= depth) {
        ttHits++;
        return it->second.score;
    }
    
    if(depth == 0){
        // Instead of static eval, call quiescence search to resolve captures
        return quiescence(game, alpha, beta, isMaximizing);
    }
    
    // NULL MOVE PRUNING
    // Try giving opponent a free move - if we're still winning, cutoff early
    const int NULL_MOVE_REDUCTION = 3;  // Search 3 plies less
    
    if (allowNullMove && depth >= NULL_MOVE_REDUCTION + 1 && !game.isInCheck()) {
        // Make null move
        game.makeNullMove();
        
        // Search with reduced depth and flipped window
        double nullScore = -alphabeta(game, depth - 1 - NULL_MOVE_REDUCTION, -beta, -beta + 1, !isMaximizing, false);
        
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
            // Checkmate: return extreme values
            eval = isMaximizing ? -100000.0 : 100000.0;
        } else {
            // Stalemate: return 0 (draw)
            eval = 0.0;
        }
        // Store in transposition table
        transpositionTable[posKey] = {eval, depth};
        return eval;
    }
    
    // Simple move ordering: captures first (MVV-LVA), then quiet moves
    // Much faster than the complex orderMoves() which makes/undoes moves
    fastOrderMoves(legalmoves);
    
    if(isMaximizing){
        //white to move - maximise eval
        //initialise max Eval to -infinity (lowest possible eval)
        double maxEval = -numeric_limits<double>::infinity();

        //run through legal moves
        int moveCount = 0;
        for(const Move& move : legalmoves){
            game.makeMoveForEngine(move);
            
            double eval;
            
            // LATE MOVE REDUCTIONS (LMR)
            // Search first few moves at full depth, reduce depth for later moves
            const int FULL_DEPTH_MOVES = 4;  // First 4 moves at full depth
            const int REDUCTION = 2;          // Reduce by 2 plies
            
            if (moveCount >= FULL_DEPTH_MOVES && depth >= 3 && !game.isInCheck()) {
                // Search at reduced depth
                eval = alphabeta(game, depth - 1 - REDUCTION, alpha, beta, false, true);
                
                // If reduced search beats beta, re-search at full depth
                if (eval > alpha) {
                    eval = alphabeta(game, depth - 1, alpha, beta, false, true);
                }
            } else {
                // Search first few moves or tactical positions at full depth
                eval = alphabeta(game, depth - 1, alpha, beta, false, true);
            }
            
            game.undoMove();
            maxEval = max(maxEval, eval);
            alpha = max(alpha, eval);
            if (beta <= alpha) {
                break; // Beta cutoff
            }
            moveCount++;
        }
        
        // Store in transposition table
        transpositionTable[posKey] = {maxEval, depth};
        return maxEval;
    } else {
        //black to move - minimise eval
        //initialise min Eval to +infinity (highest possible eval)
        double minEval = numeric_limits<double>::infinity();

        //run through legal moves
        int moveCount = 0;
        for(const Move& move : legalmoves){
            game.makeMoveForEngine(move);
            
            double eval;
            
            // LATE MOVE REDUCTIONS (LMR)
            const int FULL_DEPTH_MOVES = 4;
            const int REDUCTION = 2;
            
            if (moveCount >= FULL_DEPTH_MOVES && depth >= 3 && !game.isInCheck()) {
                // Search at reduced depth
                eval = alphabeta(game, depth - 1 - REDUCTION, alpha, beta, true, true);
                
                // If reduced search beats alpha, re-search at full depth
                if (eval < beta) {
                    eval = alphabeta(game, depth - 1, alpha, beta, true, true);
                }
            } else {
                // Search first few moves or tactical positions at full depth
                eval = alphabeta(game, depth - 1, alpha, beta, true, true);
            }
            
            game.undoMove();
            minEval = min(minEval, eval);
            beta = min(beta, eval);
            if (beta <= alpha) {
                break; // Alpha cutoff
            }
            moveCount++;
        }
        
        // Store in transposition table
        transpositionTable[posKey] = {minEval, depth};
        return minEval;
    }
}