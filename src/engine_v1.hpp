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

// V1 Engine - Baseline with fast MVV-LVA ordering and transposition table
// This version represents the optimized engine before future improvements

// Transposition table entry
struct TTEntryV1 {
    double score;
    int depth;
};

class EngineV1 {
private:
    Evaluation evaluator;
    std::unordered_map<std::string, TTEntryV1> transpositionTable;

    // Helper functions
    void fastOrderMoves(vector<Move>& moves);  // Fast MVV-LVA ordering without making moves
    
    // Search algorithm
    double alphabeta(ChessGame& game, int depth, double alpha, double beta, bool isMaximizing);

public:
    EngineV1() = default;
    EngineV1(const Evaluation& eval) : evaluator(eval) {}
    ~EngineV1() = default;

    // Main public interface
    Move getBestMove(ChessGame& game, int depth);
    
    // Optional: for debugging
    int nodesSearched = 0;
    int ttHits = 0;  // Transposition table hits
};
