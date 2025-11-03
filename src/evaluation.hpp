#pragma once
#include "board.hpp"
#include "game.hpp"
#include "moveGeneration.hpp"
#include <iostream>
#include <vector>
#include <string>

using namespace std;

// Forward declaration
class ChessGame;

class Evaluation {
private:
    // Piece values
    static constexpr double PAWN_VALUE = 1.0;
    static constexpr double KNIGHT_VALUE = 3.0;
    static constexpr double BISHOP_VALUE = 3.0;
    static constexpr double ROOK_VALUE = 5.0;
    static constexpr double QUEEN_VALUE = 9.0;
    
    // Evaluation components
    double materialCount(const ChessGame& game) const;
    double position(const ChessGame& game) const;
    double kingsafety(const ChessGame& game) const;
    double pawnStructure(const ChessGame& game) const;

public:
    Evaluation() = default;
    ~Evaluation() = default;
    
    // Main evaluation function
    // Returns positive for white advantage, negative for black advantage
    double evaluate(const ChessGame& game) const;
};