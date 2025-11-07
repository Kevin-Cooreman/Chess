#include "evaluation.hpp"
#include "game.hpp"

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
using namespace std;

//using PST's from: https://www.chessprogramming.org/PeSTO%27s_Evaluation_Function
// for now only middle game tables
// Main evaluation function
double Evaluation::evaluate(const ChessGame& game) const {
    double evaluation = 0.0;

    double mat = materialCount(game);
    double pos = 0.01*position(game);      // PSTs in centipawns, scaled down
    double king = 0.01*kingsafety(game);   // Was 16.3722, way too high!
    double pawn = 0.01*pawnStructure(game); // Was 3.32665, too high
    // Removed mating patterns and king tropism - material should dominate
    
    // Mobility bonus removed - too expensive to calculate getLegalMoves() on every eval
    // TODO: Add back more intelligently (only at root/PV nodes, or cache the count)
    
    evaluation = mat + pos + king + pawn;
    
    // Debug output - ENABLED for debugging queen capture issue
    // cout << "Eval - Mat: " << mat << " Pos: " << pos 
    // << " King: " << king << " Pawn: " << pawn << " Total: " << evaluation << endl;

    return evaluation;
}

// Count material value - optimized to use board directly instead of FEN
double Evaluation::materialCount(const ChessGame& game) const {
    double count = 0;
    
    // Iterate through the board array directly (much faster than parsing FEN)
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            int piece = board[row][col];
            if (piece == EMPTY) continue;
            
            // Get piece type (bottom 3 bits)
            int pieceType = piece & 0b0111;
            bool isWhitePiece = isWhite(piece);
            
            double pieceValue = 0;
            switch(pieceType) {
                case 0b0001: pieceValue = PAWN_VALUE; break;    // Pawn
                case 0b0011: pieceValue = KNIGHT_VALUE; break;  // Knight
                case 0b0100: pieceValue = BISHOP_VALUE; break;  // Bishop
                case 0b0010: pieceValue = ROOK_VALUE; break;    // Rook
                case 0b0101: pieceValue = QUEEN_VALUE; break;   // Queen
                case 0b0110: pieceValue = 0; break;             // King (no material value)
                default: continue;
            }
            
            // Add for white pieces, subtract for black pieces
            if (isWhitePiece) {
                count += pieceValue;
            } else {
                count -= pieceValue;
            }
        }
    }

    return count;
}

// Piece-Square Tables (PST) - bonuses for pieces on good squares
// Values are from white's perspective (flip for black)
// Values in centipawns (1 pawn = 100), will be scaled to match material values
static const int pawnPST[8][8] = {
    {  0,  0,  0,  0,  0,  0,  0,  0},  // Rank 8 (pawns can't be here)
    { 98, 134, 61, 95, 68, 126, 34, -11},  // Rank 7 (about to promote!)
    {-6,   7,  26,  31,  65,  56, 25, -20},  // Rank 6
    {-14,  13,  6,  21,  23,  12, 17, -23},  // Rank 5
    {-27,  -2,  -5,  12,  17,   6, 10, -25},  // Rank 4 (center pawns)
    {-26,  -4,  -4, -10,   3,   3, 33, -12},  // Rank 3
    {-35,  -1, -20, -23, -15,  24, 38, -22},  // Rank 2 (penalize d/e pawns not moved)
    {  0,  0,  0,  0,  0,  0,  0,  0}   // Rank 1 (pawns can't be here)
};

static const int knightPST[8][8] = {
    {-167, -89, -34, -49,  61, -97, -15, -107},  // Knights on rim are dim
    {-73, -41,  72,  36,  23,  62,   7,  -17},
    {-47,  60,  37,  65,  84, 129,  73,   44},
    {-9,  17,  19,  53,  37,  69,  18,   22},  // Knights love the center
    {-13,   4,  16,  13,  28,  19,  21,   -8},
    {-23,  -9,  12,  10,  19,  17,  25,  -16},
    {-29, -53, -12,  -3,  -1,  18, -14,  -19},
    {-105, -21, -58, -33, -17, -28, -19,  -23}
};

static const int bishopPST[8][8] = {
    {-30,  10, -90, -40, -30, -50,  10, -10},
    {-30,  30, -10, -10,  50,  80,  30, -50},  // Developed bishops - increased bonus
    {-20,  60,  60,  60,  55,  70,  60,  10},  // Good diagonals - increased bonus
    {-5,  20,  35,  70,  60,  60,  20,  10},  // Active bishops - increased bonus
    {-10,  25,  25,  45,  50,  30,  25,  15},
    {0,  30,  30,  30,  30,  45,  35,  20},
    {5,  30,  30,  5,  15,  40,  50,  10},
    {-40,  -10, -20, -30, -20, -20, -50, -30}  // Starting position - increased penalty
};

static const int rookPST[8][8] = {
    {32,  42,  32,  51, 63,  9,  31,  43},
    {27,  32,  58,  62, 80, 67,  26,  44},
    {27,  32,  58,  62, 80, 67,  26,  44},
    {-24, -11,   7,  26, 24, 35,  -8, -20},
    {-36, -26, -12,  -1,  9, -7,   6, -23},
    {-45, -25, -16, -17,  3,  0,  -5, -33},
    {-44, -16, -20,  -9, -1, 11,  -6, -71},
    {-19, -13,   1,  17, 16,  7, -37, -26}
};

static const int queenPST[8][8] = {
    {-28,   0,  29,  12,  59,  44,  43,  45},
    {-24, -39,  -5,   1, -16,  57,  28,  54},
    {-13, -17,   7,   8,  29,  56,  47,  57},
    {-27, -27, -16, -16,  -1,  17,  -2,   1},
    {-9, -26,  -9, -10,  -2,  -4,   3,  -3},
    {-14,   2, -11,  -2,  -5,   2,  14,   5},
    {-35,  -8,  11,   2,   8,  15,  -3,   1},
    {-1, -18,  -9,  10, -15, -25, -31, -50}
};

static const int kingMiddlegamePST[8][8] = {
    {-65,  23,  16, -15, -56, -34,   2,  13},
    {29,  -1, -20,  -7,  -8,  -4, -38, -29},
    {-9,  24,   2, -16, -20,   6,  22, -22},
    {-17, -20, -12, -27, -30, -25, -14, -36},
    {-49,  -1, -27, -39, -46, -44, -33, -51},
    {-14, -14, -22, -46, -44, -30, -15, -27},
    {1,   7,  -8, -64, -43, -16,   9,   8},
    {-15,  36,  12, -54,   8, -28,  24,  14}
};

// Evaluate piece positioning using piece-square tables - optimized to use board directly
double Evaluation::position(const ChessGame& game) const {
    double positionValue = 0.0;
    
    // Iterate through board array directly (much faster than parsing FEN)
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            int piece = board[row][col];
            if (piece == EMPTY) continue;
            
            int pieceType = piece & 0b0111;
            bool isWhitePiece = isWhite(piece);
            
            // For black pieces, flip the row to get correct PST index
            int pstRow = isWhitePiece ? row : (7 - row);
            
            double pieceValue = 0;
            // Use piece-square tables (values are in centipawns, so divide by 100 to match pawn=1 scale)
            switch(pieceType) {
                case 0b0001: pieceValue = pawnPST[pstRow][col] / 100.0; break;      // Pawn
                case 0b0011: pieceValue = knightPST[pstRow][col] / 100.0; break;    // Knight
                case 0b0100: pieceValue = bishopPST[pstRow][col] / 100.0; break;    // Bishop
                case 0b0010: pieceValue = rookPST[pstRow][col] / 100.0; break;      // Rook
                case 0b0101: pieceValue = queenPST[pstRow][col] / 100.0; break;     // Queen
                case 0b0110: pieceValue = kingMiddlegamePST[pstRow][col] / 100.0; break; // King
                default: continue;
            }
            
            // Add for white pieces, subtract for black pieces
            if (isWhitePiece) {
                positionValue += pieceValue;
            } else {
                positionValue -= pieceValue;
            }
        }
    }

    return positionValue;
}

// king safety evaluation - simplified version, optimized to use board directly
double Evaluation::kingsafety(const ChessGame& game) const {
    double kingSafetyValue = 0.0;
    
    // Find kings by iterating through board
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            int piece = board[row][col];
            if (piece == EMPTY) continue;
            
            int pieceType = piece & 0b0111;
            if (pieceType != 0b0110) continue;  // Only process kings
            
            bool isWhitePiece = isWhite(piece);
            double safetyPenalty = 0.0;
            
            // Kings are safer on back rank and in corners
            if (isWhitePiece) {
                // White king: safer on row 7 (back rank)
                safetyPenalty = (7 - row) * 0.02;  // Penalty for advancing
                // Bonus for being castled (on g or c file on back rank)
                if (row == 7 && (col == 6 || col == 2)) {
                    safetyPenalty -= 0.02;
                }
                kingSafetyValue -= safetyPenalty;
            } else {
                // Black king: safer on row 0 (back rank)
                safetyPenalty = row * 0.02;  // Penalty for advancing
                // Bonus for being castled
                if (row == 0 && (col == 6 || col == 2)) {
                    safetyPenalty -= 0.02;
                }
                kingSafetyValue += safetyPenalty;
            }
        }
    }

    return kingSafetyValue;
}

// Pawn structure evaluation - optimized to use board directly
double Evaluation::pawnStructure(const ChessGame& game) const {
    double pawnStructureValue = 0.0;
    
    // Iterate through board to find pawns
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            int piece = board[row][col];
            if (piece == EMPTY) continue;
            
            int pieceType = piece & 0b0111;
            if (pieceType != 0b0001) continue;  // Only process pawns
            
            double pieceValue = 0.0;
            bool isWhitePawn = isWhite(piece);
            
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
                pieceValue += (8 - distanceToPromotion) * 0.01;
            }
            
            // Check for doubled pawns (penalty)
            vector<Move> fileMoves = isWhitePawn ? generateDownMoves(row, col) : generateUpMoves(row, col);
            for(const Move& move : fileMoves) {
                int checkPiece = board[move.targetRow][move.targetColumn];
                if((checkPiece & 0b0111) == 0b0001 && isWhite(checkPiece) == isWhitePawn) {
                    pieceValue -= 0.05; // Penalty for doubled pawns
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
            
            // Heavy penalty for isolated pawns, especially if advanced
            if(!hasSupport) {
                pieceValue -= 0.1;  // Base penalty increased from 0.015
                
                // Additional penalty for isolated pawns on the edges (a/h files)
                if(col == 0 || col == 7) {
                    pieceValue -= 0.2;  // Edge pawns are especially weak when isolated
                }
                
                // Extra penalty if the isolated pawn has advanced (more vulnerable)
                int advancement = isWhitePawn ? row : (7 - row);
                if(advancement > 2) {
                    pieceValue -= 0.15 * (advancement - 2);  // Penalty grows with advancement
                }
            }
            
            // Add for white pawns, subtract for black pawns
            if(isWhitePawn) {
                pawnStructureValue += pieceValue;
            } else {
                pawnStructureValue -= pieceValue;
            }
        }
    }

    return pawnStructureValue;
}