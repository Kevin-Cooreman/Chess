#pragma once
#include "board.hpp"
#include "game.hpp"
#include "moveGeneration.hpp"
#include <iostream>
#include <vector>

using namespace std;

const double PAWN_VALUE = 1.0;
const double KNIGHT_VALUE = 3.0;
const double BISHOP_VALUE = 3.0;
const double ROOK_VALUE = 5.0;
const double QUEEN_VALUE = 9.0;

// Forward declaration
class ChessGame;

//evaluation, positive for white , negative for black
// => if black minimize score, if white maximise
double evaluation(const ChessGame& game);

//returns a double fo type consistency
//gets current game state and calcualtes value
double materialCount(const ChessGame& game);

//piece positioning
double position(const ChessGame& game);

//king safety
double kingsafety(const ChessGame& game);

//pawn structure
double pawnStructure(const ChessGame& game);