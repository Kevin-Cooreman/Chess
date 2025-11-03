#include "evaluation.hpp"
#include "game.hpp"

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
using namespace std;

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
    
}