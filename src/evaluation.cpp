#include "evaluation.hpp"
#include "game.hpp"

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
using namespace std;

double evaluation(const ChessGame& game){
    double evaluation = 0.0;

    evaluation += materialCount(game);
    evaluation += position(game);

    return evaluation;
}

//count all pieces
//for each white piece add piece value for each black piece subtract
double materialCount(const ChessGame& game){
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


// for all regular pieces => most amount of moves penalty if can be taken/ not defended
// bonus for putting king in check
// infinite bonus for delivering checkmate
double position(const ChessGame& game){
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
            case 'p': pieceValue = generatePawnMoves(row, col).size() * 0.1; break;  // Pawn mobility less important
            case 'n': pieceValue = generateKnightMoves(row, col).size() * 0.1; break;
            case 'b': pieceValue = generateBishopMoves(row, col).size() * 0.05; break;
            case 'r': pieceValue = generateRookMoves(row, col).size() * 0.05; break;
            case 'q': pieceValue = generateQueenMoves(row, col).size() * 0.05; break;
            case 'k': pieceValue = generateKingMoves(row, col).size() * 0.05; break;  // King mobility matters for endgame
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