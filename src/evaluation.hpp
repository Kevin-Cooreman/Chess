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

//evaluation, positive for white , negative for black
// => if black minimize score, if white maximise
double evaluation();

//returns a double fo type consistency
//gets current game state and calcualtes value
double materialDount();

//piece positioning
double position();

//king safety
double kingsafety();

//pawn structure
double pawnStructure();