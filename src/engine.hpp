#pragma once
#include "board.hpp"
#include "game.hpp"
#include "evaluation.hpp"
#include "moveGeneration.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <limits>

class Engine {
private:
    Evaluation evaluator;

    // Search algorithm
    double minimax(ChessGame& game, int depth, bool isMaximizing);
    double alphabeta(ChessGame& game, int depth, double alpha, double beta, bool isMaximizing);

public:
    Engine() = default;
    Engine(const Evaluation& eval) : evaluator(eval) {}
    ~Engine() = default;

    // Main public interface
    Move getBestMove(ChessGame& game, int depth);
    
    // Optional: for debugging
    int nodesSearched = 0;
};

