#pragma once
/*made with help of github copilot agent*/
#include <iostream>
#include <array>
#include <string>

using namespace std;

//startposition using FEN
const string startingPosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";

//empty square
const int EMPTY = 0b0000;

//pieces: first(LSB) 3 bits = piece, last bit(MSB) = colour
const int WHITE_PAWN = 0b0001;
const int WHITE_ROOK = 0b0010;
const int WHITE_KNIGHT = 0b0011;
const int WHITE_BISHOP = 0b0100;
const int WHITE_QUEEN = 0b0101;
const int WHITE_KING = 0b0110;

const int BLACK_PAWN = 0b1001;
const int BLACK_ROOK = 0b1010;
const int BLACK_KNIGHT = 0b1011;
const int BLACK_BISHOP = 0b1100;
const int BLACK_QUEEN = 0b1101;
const int BLACK_KING = 0b1110;

extern int board[8][8];  // Declaration only

// Game state tracking for special moves
extern bool whiteKingMoved;
extern bool blackKingMoved;
extern bool whiteKingsideRookMoved;
extern bool whiteQueensideRookMoved;
extern bool blackKingsideRookMoved;
extern bool blackQueensideRookMoved;

// En passant target square (-1 means no en passant available)
extern int enPassantTargetRow;
extern int enPassantTargetCol;

//simple helper functions
inline bool isEmpty(int square){return square == 0;}
inline bool isWhite(int square){return square > 0 && !(square & 0b1000);}
inline bool isBlack(int square){return square & 0b1000;} // Direct bit check
inline bool sameColour(int square1, int square2){
    return  (isWhite(square1) && isWhite(square2)) ||
            (isBlack(square1) && isBlack(square2));
}

//basic board functions
void initBoard(); //initialises empty board

void setupStartingPosition(); //sets up starting position

void printBoard(); //prints the board

char pieceToChar(int piece); //converts a piece to it's character representation

int charToPiece(char piece);