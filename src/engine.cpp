#include "engine.hpp"
#include <algorithm>
#include <limits>

// Get the best move for the current position
Move Engine::getBestMove(ChessGame& game, int depth) {
    std::vector<Move> legalMoves = game.getLegalMoves();
    
    if (legalMoves.empty()) {
        return Move(-1, -1, -1, -1); // No legal moves
    }
    
    // TODO: Search through moves and pick best one
    // For now, just return first legal move
    return legalMoves[0];
}

// Minimax algorithm
double Engine::minimax(ChessGame& game, int depth, bool isMaximizing) {
    // TODO: Implement minimax search
    return evaluator.evaluate(game);
}

// Alpha-beta pruning (optimized minimax) - implement later
double Engine::alphabeta(ChessGame& game, int depth, double alpha, double beta, bool isMaximizing) {
    // TODO: Implement alpha-beta pruning
    return minimax(game, depth, isMaximizing);
}