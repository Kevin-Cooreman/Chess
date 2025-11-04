#include "engine.hpp"
#include <algorithm>
#include <limits>

using namespace std;

// Get the best move for the current position
Move Engine::getBestMove(ChessGame& game, int depth) {
    nodesSearched = 0;  // Reset counter at start of search
    ttHits = 0;  // Reset TT hits counter
    transpositionTable.clear();  // Clear transposition table for new search
    
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
    
    if (isWhiteTurn) {
        // White wants to maximize evaluation
        double bestEval = -numeric_limits<double>::infinity();
        
        for (const Move& move : validatedMoves) {
            game.makeMoveForEngine(move);
            
            double eval = alphabeta(game, depth - 1, -numeric_limits<double>::infinity(), 
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
            double eval = alphabeta(game, depth - 1, -numeric_limits<double>::infinity(), 
                                   numeric_limits<double>::infinity(), true);
            game.undoMove();
            
            if (eval < bestEval) {
                bestEval = eval;
                bestMove = move;
            }
        }
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

// Alpha-beta pruning (optimized minimax)
double Engine::alphabeta(ChessGame& game, int depth, double alpha, double beta, bool isMaximizing) {
    nodesSearched++;  // Count this node
    
    // Check transposition table BEFORE generating moves (expensive operation)
    string posKey = game.getPositionKey();
    auto it = transpositionTable.find(posKey);
    if (it != transpositionTable.end() && it->second.depth >= depth) {
        ttHits++;
        return it->second.score;
    }
    
    if(depth == 0){
        double eval = evaluator.evaluate(game);
        // Store in transposition table
        transpositionTable[posKey] = {eval, depth};
        return eval;
    }

    vector<Move> legalmoves = game.getLegalMoves();
    
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
        for(const Move& move : legalmoves){
            game.makeMoveForEngine(move);
            double eval = alphabeta(game, depth - 1, alpha, beta, false);
            game.undoMove();
            maxEval = max(maxEval, eval);
            alpha = max(alpha, eval);
            if (beta <= alpha) {
                break; // Beta cutoff
            }
        }
        
        // Store in transposition table
        transpositionTable[posKey] = {maxEval, depth};
        return maxEval;
    } else {
        //black to move - minimise eval
        //initialise min Eval to +infinity (highest possible eval)
        double minEval = numeric_limits<double>::infinity();

        //run through legal moves
        for(const Move& move : legalmoves){
            game.makeMoveForEngine(move);
            double eval = alphabeta(game, depth - 1, alpha, beta, true);
            game.undoMove();
            minEval = min(minEval, eval);
            beta = min(beta, eval);
            if (beta <= alpha) {
                break; // Alpha cutoff
            }
        }
        
        // Store in transposition table
        transpositionTable[posKey] = {minEval, depth};
        return minEval;
    }
}