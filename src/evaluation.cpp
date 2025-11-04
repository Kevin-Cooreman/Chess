#include "evaluation.hpp"
#include "game.hpp"

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
using namespace std;

// Main evaluation function
double Evaluation::evaluate(const ChessGame& game) const {
    double evaluation = 0.0;

    evaluation += 10*materialCount(game);
    evaluation += 3*position(game);
    evaluation += 3*kingsafety(game);
    evaluation += 0.05*pawnStructure(game);

    return evaluation;
}

// Count material value
double Evaluation::materialCount(const ChessGame& game) const {
    double count = 0;
    string fen = game.getCurrentFEN();

    // Parse only the piece placement part (before the first space)
    size_t spacePos = fen.find(' ');
    string piecePlacement = fen.substr(0, spacePos);

    // Count piece values from FEN
    for(char c : piecePlacement) {
        if(c == '/' || isdigit(c)) {
            continue; // Skip rank separators and empty squares
        }
        
        // Determine piece value
        double pieceValue = 0;
        char piece = tolower(c);
        
        switch(piece) {
            case 'p': pieceValue = PAWN_VALUE; break;
            case 'n': pieceValue = KNIGHT_VALUE; break;
            case 'b': pieceValue = BISHOP_VALUE; break;
            case 'r': pieceValue = ROOK_VALUE; break;
            case 'q': pieceValue = QUEEN_VALUE; break;
            case 'k': pieceValue = 0; break; // King has no material value
            default: continue;
        }
        
        // Add for white pieces (uppercase), subtract for black pieces (lowercase)
        if(isupper(c)) {
            count += pieceValue;
        } else {
            count -= pieceValue;
        }
    }

    return count;
}

// Piece-Square Tables (PST) - bonuses for pieces on good squares
// Values are from white's perspective (flip for black)
static const int pawnPST[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    {50, 50, 50, 50, 50, 50, 50, 50},
    {10, 10, 20, 30, 30, 20, 10, 10},
    { 5,  5, 10, 25, 25, 10,  5,  5},
    { 0,  0,  0, 20, 20,  0,  0,  0},
    { 5, -5,-10,  0,  0,-10, -5,  5},
    { 5, 10, 10,-20,-20, 10, 10,  5},
    { 0,  0,  0,  0,  0,  0,  0,  0}
};

static const int knightPST[8][8] = {
    {-50,-40,-30,-30,-30,-30,-40,-50},
    {-40,-20,  0,  0,  0,  0,-20,-40},
    {-30,  0, 10, 15, 15, 10,  0,-30},
    {-30,  5, 15, 20, 20, 15,  5,-30},
    {-30,  0, 15, 20, 20, 15,  0,-30},
    {-30,  5, 10, 15, 15, 10,  5,-30},
    {-40,-20,  0,  5,  5,  0,-20,-40},
    {-50,-40,-30,-30,-30,-30,-40,-50}
};

static const int bishopPST[8][8] = {
    {-20,-10,-10,-10,-10,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5, 10, 10,  5,  0,-10},
    {-10,  5,  5, 10, 10,  5,  5,-10},
    {-10,  0, 10, 10, 10, 10,  0,-10},
    {-10, 10, 10, 10, 10, 10, 10,-10},
    {-10,  5,  0,  0,  0,  0,  5,-10},
    {-20,-10,-10,-10,-10,-10,-10,-20}
};

static const int rookPST[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 5, 10, 10, 10, 10, 10, 10,  5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    { 0,  0,  0,  5,  5,  0,  0,  0}
};

static const int queenPST[8][8] = {
    {-20,-10,-10, -5, -5,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5,  5,  5,  5,  0,-10},
    { -5,  0,  5,  5,  5,  5,  0, -5},
    {  0,  0,  5,  5,  5,  5,  0, -5},
    {-10,  5,  5,  5,  5,  5,  0,-10},
    {-10,  0,  5,  0,  0,  0,  0,-10},
    {-20,-10,-10, -5, -5,-10,-10,-20}
};

static const int kingMiddlegamePST[8][8] = {
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-20,-30,-30,-40,-40,-30,-30,-20},
    {-10,-20,-20,-20,-20,-20,-20,-10},
    { 20, 20,  0,  0,  0,  0, 20, 20},
    { 20, 30, 10,  0,  0, 10, 30, 20}
};

// Evaluate piece positioning using piece-square tables
double Evaluation::position(const ChessGame& game) const {
    double positionValue = 0.0;
    string fen = game.getCurrentFEN();

    size_t spacePos = fen.find(' ');
    string piecePlacement = fen.substr(0, spacePos);

    int row = 0, col = 0;

    for(char c : piecePlacement) {
        if(c == '/') {
            row++;      // Move to next rank
            col = 0;    // Reset to file 'a'
            continue;
        }
        else if(isdigit(c)) {
            col += (c - '0');  // Skip empty squares
            continue;
        }
        
        // Now we have a piece at position [row][col]
        double pieceValue = 0;
        char piece = tolower(c);
        bool isWhite = isupper(c);
        
        // For black pieces, flip the row to get correct PST index
        int pstRow = isWhite ? row : (7 - row);
        
        // Use piece-square tables instead of generating moves
        switch(piece) {
            case 'p': pieceValue = pawnPST[pstRow][col] * 0.01; break;
            case 'n': pieceValue = knightPST[pstRow][col] * 0.01; break;
            case 'b': pieceValue = bishopPST[pstRow][col] * 0.01; break;
            case 'r': pieceValue = rookPST[pstRow][col] * 0.01; break;
            case 'q': pieceValue = queenPST[pstRow][col] * 0.01; break;
            case 'k': pieceValue = kingMiddlegamePST[pstRow][col] * 0.01; break;
            default: 
                col++;
                continue;
        }
        
        // Add for white pieces, subtract for black pieces
        if(isWhite) {
            positionValue += pieceValue;
        } else {
            positionValue -= pieceValue;
        }
        
        col++;  // Move to next column
    }

    return positionValue;
}

// king safety evaluation - simplified version
double Evaluation::kingsafety(const ChessGame& game) const {
    double kingSafetyValue = 0.0;
    string fen = game.getCurrentFEN();

    size_t spacePos = fen.find(' ');
    string piecePlacement = fen.substr(0, spacePos);

    int row = 0, col = 0;

    for(char c : piecePlacement) {
        if(c == '/') {
            row++;      // Move to next rank
            col = 0;    // Reset to file 'a'
            continue;
        }
        else if(isdigit(c)) {
            col += (c - '0');  // Skip empty squares
            continue;
        }
        
        char piece = tolower(c);

        if(piece == 'k'){
            bool isWhite = isupper(c);
            double safetyPenalty = 0.0;
            
            // Kings are safer on back rank and in corners
            if (isWhite) {
                // White king: safer on row 7 (back rank)
                safetyPenalty = (7 - row) * 2.0;  // Penalty for advancing
                // Bonus for being castled (on g or c file on back rank)
                if (row == 7 && (col == 6 || col == 2)) {
                    safetyPenalty -= 5.0;
                }
            } else {
                // Black king: safer on row 0 (back rank)
                safetyPenalty = row * 2.0;  // Penalty for advancing
                // Bonus for being castled
                if (row == 0 && (col == 6 || col == 2)) {
                    safetyPenalty -= 5.0;
                }
            }
            
            // More exposure = bad for that side
            if(isWhite) {
                kingSafetyValue -= safetyPenalty;
            } else {
                kingSafetyValue += safetyPenalty;
            }
        }
        
        col++;  // Move to next column
    }

    return kingSafetyValue;
}

// Placeholder for pawn structure evaluation
double Evaluation::pawnStructure(const ChessGame& game) const {
    double pawnStructureValue = 0.0;
    string fen = game.getCurrentFEN();

    size_t spacePos = fen.find(' ');
    string piecePlacement = fen.substr(0, spacePos);

    int row = 0, col = 0;

    for(char c : piecePlacement) {
        if(c == '/') {
            row++;      // Move to next rank
            col = 0;    // Reset to file 'a'
            continue;
        }
        else if(isdigit(c)) {
            col += (c - '0');  // Skip empty squares
            continue;
        }
        
        // Now we have a piece at position [row][col]
        char piece = tolower(c);

        if(piece == 'p'){
            double pieceValue = 0.0;
            bool isWhitePawn = isupper(c);
            
            // Check for passed pawn (no enemy pawns ahead in file or adjacent files)
            vector<Move> aheadMoves = isWhitePawn ? generateUpMoves(row, col) : generateDownMoves(row, col);
            bool hasEnemyAhead = false;
            
            // Check if there are enemy pawns ahead in this file or adjacent files
            for(const Move& move : aheadMoves) {
                int targetRow = move.targetRow;
                // Check current file and adjacent files
                for(int fileOffset = -1; fileOffset <= 1; fileOffset++) {
                    int checkCol = col + fileOffset;
                    if(checkCol >= 0 && checkCol < 8 && board[targetRow][checkCol] != EMPTY) {
                        int checkPiece = board[targetRow][checkCol];
                        // Check if it's an enemy pawn
                        if((checkPiece & 0b0111) == 0b0001 && isWhite(checkPiece) != isWhitePawn) {
                            hasEnemyAhead = true;
                            break;
                        }
                    }
                }
                if(hasEnemyAhead) break;
            }
            
            // Reward passed pawns (bonus increases closer to promotion)
            if(!hasEnemyAhead) {
                int distanceToPromotion = isWhitePawn ? row : (7 - row);
                pieceValue += (8 - distanceToPromotion) * 0.05; // 0.05 to 0.4 bonus
            }
            
            // Check for doubled pawns (penalty)
            vector<Move> fileMoves = isWhitePawn ? generateDownMoves(row, col) : generateUpMoves(row, col);
            for(const Move& move : fileMoves) {
                int checkPiece = board[move.targetRow][move.targetColumn];
                if((checkPiece & 0b0111) == 0b0001 && isWhite(checkPiece) == isWhitePawn) {
                    pieceValue -= 0.2; // Penalty for doubled pawns
                    break;
                }
            }
            
            // Check for isolated pawns (no friendly pawns on adjacent files)
            bool hasSupport = false;
            for(int fileOffset = -1; fileOffset <= 1; fileOffset += 2) { // Check left and right files only
                int checkCol = col + fileOffset;
                if(checkCol >= 0 && checkCol < 8) {
                    // Check entire file for friendly pawns
                    for(int checkRow = 0; checkRow < 8; checkRow++) {
                        int checkPiece = board[checkRow][checkCol];
                        if((checkPiece & 0b0111) == 0b0001 && isWhite(checkPiece) == isWhitePawn) {
                            hasSupport = true;
                            break;
                        }
                    }
                }
                if(hasSupport) break;
            }
            
            // Penalty for isolated pawns
            if(!hasSupport) {
                pieceValue -= 0.15;
            }
            
            // Add for white pieces (uppercase), subtract for black pieces (lowercase)
            if(isupper(c)) {
                pawnStructureValue += pieceValue;
            } else {
                pawnStructureValue -= pieceValue;
            }
        }
        
        col++;  // Move to next column
    }

    return pawnStructureValue;
}