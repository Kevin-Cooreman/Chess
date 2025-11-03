#include "engine.hpp"
#include <algorithm>
#include <limits>

using namespace std;

// Get the best move for the current position
Move Engine::getBestMove(ChessGame& game, int depth) {
    vector<Move> legalMoves = game.getLegalMoves();
    
    if (legalMoves.empty()) {
        return Move(-1, -1, -1, -1); // No legal moves
    }
    
    bool isWhiteTurn = game.isWhiteToMove();
    Move bestMove = legalMoves[0];
    
    if (isWhiteTurn) {
        // White wants to maximize evaluation
        double bestEval = -numeric_limits<double>::infinity();
        
        for (const Move& move : legalMoves) {
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
        
        for (const Move& move : legalMoves) {
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

// Alpha-beta pruning (optimized minimax) - implement later
double Engine::alphabeta(ChessGame& game, int depth, double alpha, double beta, bool isMaximizing) {
    
    if(depth == 0){
        return evaluator.evaluate(game);
    }

    vector<Move> legalmoves = game.getLegalMoves();
    
    // If no legal moves, it's checkmate or stalemate
    if(legalmoves.empty()) {
        if(game.isInCheck()) {
            // Checkmate: return extreme values
            return isMaximizing ? -100000.0 : 100000.0;
        } else {
            // Stalemate: return 0 (draw)
            return 0.0;
        }
    }
    
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
        
        return minEval;
    }
}