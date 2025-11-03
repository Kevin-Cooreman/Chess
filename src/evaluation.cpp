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
    // Can add more components later:
    evaluation += kingsafety(game);
    // evaluation += pawnStructure(game);

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
    // TODO: Implement pawn structure evaluation
    (void)game; // Suppress unused parameter warning
    return 0.0;
}