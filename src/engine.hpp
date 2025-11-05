#pragma once
#include "board.hpp"
#include "game.hpp"
#include "evaluation.hpp"
#include "moveGeneration.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <unordered_map>

// Transposition table entry
struct TTEntry {
    double score;
    int depth;
};

class Engine {
private:
    Evaluation evaluator;
    std::unordered_map<std::string, TTEntry> transpositionTable;
    Move pvMove = Move(-1, -1, -1, -1);  // Initialize to invalid move

    // Helper functions
    void fastOrderMoves(vector<Move>& moves);  // Fast MVV-LVA ordering without making moves
    vector<Move> generateCaptureMoves(ChessGame& game);  // Generate only capture moves for quiescence

    // Search algorithm
    double alphabeta(ChessGame& game, int depth, double alpha, double beta, bool isMaximizing, bool allowNullMove = true);
    double quiescence(ChessGame& game, double alpha, double beta, bool isMaximizing, int qDepth = 0);  // Quiescence search

public:
    Engine() = default;
    Engine(const Evaluation& eval) : evaluator(eval) {}
    ~Engine() = default;

    // Main public interface
    Move getBestMove(ChessGame& game, int depth);
    
    // Optional: for debugging
    int nodesSearched = 0;
    int ttHits = 0;  // Transposition table hits
};

