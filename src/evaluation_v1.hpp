// evaluation_v1.hpp - Baseline version for benchmarking
#pragma once
#include "board.hpp"
#include "game.hpp"
#include <iostream>
#include <vector>
#include <string>

using namespace std;

class ChessGame;

class EvaluationV1 {
protected:
    static constexpr double PAWN_VALUE = 1.0;
    static constexpr double KNIGHT_VALUE = 3.0;
    static constexpr double BISHOP_VALUE = 3.0;
    static constexpr double ROOK_VALUE = 5.0;
    static constexpr double QUEEN_VALUE = 9.0;
    
    double materialCount(const ChessGame& game) const;
    double position(const ChessGame& game) const;
    double kingsafety(const ChessGame& game) const;
    double pawnStructure(const ChessGame& game) const;

public:
    EvaluationV1() = default;
    ~EvaluationV1() = default;
    
    double evaluate(const ChessGame& game) const;
};
