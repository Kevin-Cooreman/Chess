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
    
    // TODO: Search through moves and pick best one
    // For now, just return first legal move
    return legalMoves[0];
}

// Minimax algorithm
double Engine::minimax(ChessGame& game, int depth, bool isMaximizing) {
    if(depth == 0){
        return evaluator.evaluate(game);
    }

    vector<Move> legalmoves = game.getLegalMoves();
    
    if(isMaximizing){
        //white to move - maximise eval
        //initialise max Eval to -infinity (lowest possible eval)
        double maxEval = -numeric_limits<double>::infinity();

        //run through legal moves
        for(const Move& move : legalmoves){
            game.makeMoveForEngine(move);
            double eval = minimax(game, depth - 1, false);
            game.undoMove();
            maxEval = max(maxEval, eval);
        }
        
        return maxEval;
    } else {
        //black to move - minimise eval
        //initialise min Eval to +infinity (highest possible eval)
        double minEval = numeric_limits<double>::infinity();

        //run through legal moves
        for(const Move& move : legalmoves){
            game.makeMoveForEngine(move);
            double eval = minimax(game, depth - 1, true);
            game.undoMove();
            minEval = min(minEval, eval);
        }
        
        return minEval;
    }
}

// Alpha-beta pruning (optimized minimax) - implement later
double Engine::alphabeta(ChessGame& game, int depth, double alpha, double beta, bool isMaximizing) {
    // TODO: Implement alpha-beta pruning
    return minimax(game, depth, isMaximizing);
}