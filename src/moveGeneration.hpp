/*generate legal moves for each piece
- possible moves
- check if blocked
- check if it can take
- complex moves like castle and en passant*/

#include "board.hpp"
#include <iostream>
#include <vector>

using namespace std;

struct Move{
    int startRow;
    int startColumn;
    int targetRow;
    int targetColumn;
};

bool isEnemy(int targetPiece, int currentPlayerPiece);

//generate possible moves for each piece
vector<Move> generateRookMoves(int sRow, int sCol);
vector<Move> generateBishopMoves(int sRow, int sCol);
vector<Move> generateQueenMoves(int sRow, int sCol);
vector<Move> generateKingMoves(int sRow, int sCol);
vector<Move> generateKnightMoves(int sRow, int sCol);
vector<Move> generatePawnMoves(int sRow, int sCol);
