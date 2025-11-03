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

    evaluation += materialCount(game);
    evaluation += position(game);
    evaluation += kingsafety(game);
    evaluation += pawnStructure(game);

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

// Evaluate piece positioning based on mobility
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
        
        // evaluate position by nr of moves available
        switch(piece) {
            case 'p': pieceValue = generatePawnMoves(row, col).size() * 0.1; break;
            case 'n': pieceValue = generateKnightMoves(row, col).size() * 0.1; break;
            case 'b': pieceValue = generateBishopMoves(row, col).size() * 0.05; break;
            case 'r': pieceValue = generateRookMoves(row, col).size() * 0.05; break;
            case 'q': pieceValue = generateQueenMoves(row, col).size() * 0.05; break;
            case 'k': pieceValue = generateKingMoves(row, col).size() * 0.05; break;
            default: 
                col++;
                continue;
        }
        
        // Add for white pieces (uppercase), subtract for black pieces (lowercase)
        if(isupper(c)) {
            positionValue += pieceValue;
        } else {
            positionValue -= pieceValue;
        }
        
        col++;  // Move to next column
    }

    return positionValue;
}

// king safety evaluation
double Evaluation::kingsafety(const ChessGame& game) const {
    // TODO: Implement king safety evaluation
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
        
        // Now we have a piece at position [row][col]
        double pieceValue = 0;
        char piece = tolower(c);

        if(piece == 'k'){
            int ifKingWasQueen = generateQueenMoves(row, col).size();
            pieceValue = ifKingWasQueen * 0.5; // Penalty for exposed king
        }

        else{
            col++;
            continue;
        }
        
        // More mobility = more exposed = bad for that side
        // Subtract for white king exposure, add for black king exposure
        if(isupper(c)) {
            kingSafetyValue -= pieceValue;  // White king exposed is bad
        } else {
            kingSafetyValue += pieceValue;  // Black king exposed is good for white
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
                pieceValue += (8 - distanceToPromotion) * 0.1; // 0.1 to 0.8 bonus
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