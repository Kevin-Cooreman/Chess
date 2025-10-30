#pragma once
/*generate legal moves for each piece
- possible moves
- check if blocked
- check if it can take
- complex moves like castle and en passant*/

#include "board.hpp"
#include <iostream>
#include <vector>

using namespace std;

enum MoveType {
    NORMAL,
    CASTLING_KINGSIDE,
    CASTLING_QUEENSIDE,
    EN_PASSANT,
    PAWN_PROMOTION
};

struct Move{
    int startRow;
    int startColumn;
    int targetRow;
    int targetColumn;
    MoveType moveType;
    int promotionPiece; // For pawn promotion (QUEEN, ROOK, BISHOP, KNIGHT)
    
    // Constructor for normal moves
    Move(int sR, int sC, int tR, int tC, MoveType type = NORMAL, int promo = 0) 
        : startRow(sR), startColumn(sC), targetRow(tR), targetColumn(tC), 
          moveType(type), promotionPiece(promo) {}
};

bool isEnemy(int targetPiece, int currentPlayerPiece);

//generate possible moves for each piece
vector<Move> generateRookMoves(int sRow, int sCol);
vector<Move> generateBishopMoves(int sRow, int sCol);
vector<Move> generateQueenMoves(int sRow, int sCol);
vector<Move> generateKingMoves(int sRow, int sCol);
vector<Move> generateKnightMoves(int sRow, int sCol);
vector<Move> generatePawnMoves(int sRow, int sCol);

// Helper functions for legal move validation
bool isSquareAttacked(int row, int col, bool byWhite);
bool isKingInCheck(bool whiteKing);
bool isMoveLegal(const Move& move);

// Main function - generates only legal moves
vector<Move> generateLegalMoves(bool isWhiteTurn);

// Helper to generate moves for any piece at a position
vector<Move> generateMovesForPiece(int row, int col);

// Generate basic moves without special moves (for attack detection)
vector<Move> generateBasicMovesForPiece(int row, int col);

// Special move functions
vector<Move> generateCastlingMoves(bool isWhite);
vector<Move> generateEnPassantMoves(int sRow, int sCol);
vector<Move> generatePawnPromotionMoves(int sRow, int sCol, int targetRow, int targetCol);

// Game state management
void makeMove(const Move& move);
void updateGameState(const Move& move);
