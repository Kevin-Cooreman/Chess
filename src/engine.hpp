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
#include <cstdint>
#include <random>

// Transposition table entry
enum class TTBound : int {
    EXACT = 0,
    LOWER = 1,
    UPPER = 2
};

struct TTEntry {
    double score;
    int depth;
    TTBound bound = TTBound::EXACT; // Whether the stored score is exact, a lower bound or an upper bound
    int mateDistance = 0; // If this entry represents a mate score, store mate-in-N (plies) here. 0 = not a mate
};

class Engine {
private:
    Evaluation evaluator;
    std::unordered_map<uint64_t, TTEntry> transpositionTable;  // Changed from string to uint64_t
    Move pvMove = Move(-1, -1, -1, -1);  // Initialize to invalid move

    // Helper functions
    void fastOrderMoves(vector<Move>& moves);  // Fast MVV-LVA ordering without making moves
    void orderRootMoves(ChessGame& game, vector<Move>& moves); // Order root moves, preferring checks/mates
    vector<Move> generateCaptureMoves(ChessGame& game);  // Generate only capture moves for quiescence

    // Search algorithm
    // 'ply' is the number of plies from the root (used to prefer shorter mates)
    double alphabeta(ChessGame& game, int depth, double alpha, double beta, bool isMaximizing, bool allowNullMove = true, int ply = 0);
    double quiescence(ChessGame& game, double alpha, double beta, bool isMaximizing, int qDepth = 0);  // Quiescence search
    // Root mate prover: try to prove mate within maxDepth plies. If a mate is found,
    // returns true and sets outMove to the mating root move.
    bool rootMateProver(ChessGame& game, int maxDepth, Move& outMove);
    bool canForceMate(ChessGame& game, int depthLeft, bool attackerIsWhite);

public:
    Engine() = default;
    Engine(const Evaluation& eval) : evaluator(eval) {}
    ~Engine() = default;

    // Main public interface
    Move getBestMove(ChessGame& game, int depth);
    // Set RNG seed used for root move randomization (opening variety)
    static void setRngSeed(uint64_t seed);
    
    // Optional: for debugging
    int nodesSearched = 0;
    int ttHits = 0;  // Transposition table hits
};

