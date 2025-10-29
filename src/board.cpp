#include "board.hpp"
#include <iostream>
#include <array>
#include <string>

using namespace std;

int board[8][8];

// initialise empty board
void initBoard(){
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            board[i][j] = EMPTY;
        }
    }
}

//converts a piece to it's character representation
char pieceToChar(int piece){
    switch(piece){
        //WHITE
        case 0b0001: return 'P';
        case 0b0010: return 'R';
        case 0b0011: return 'N';
        case 0b0100: return 'B';
        case 0b0101: return 'Q';
        case 0b0110: return 'K';
        //BLACK
        case 0b1001: return 'p';
        case 0b1010: return 'r';
        case 0b1011: return 'n';
        case 0b1100: return 'b';
        case 0b1101: return 'q';
        case 0b1110: return 'k';
        //DEFAULT
        default: return ' ';
    }
}

int charToPiece(char piece){
        switch(piece){
        //WHITE
        case 'P': return WHITE_PAWN;
        case 'R': return WHITE_ROOK;
        case 'N': return WHITE_KNIGHT;
        case 'B': return WHITE_BISHOP;
        case 'Q': return WHITE_QUEEN;
        case 'K': return WHITE_KING;
        //BLACK
        case 'p': return BLACK_PAWN;
        case 'r': return BLACK_ROOK;
        case 'n': return BLACK_KNIGHT;
        case 'b': return BLACK_BISHOP;
        case 'q': return BLACK_QUEEN;
        case 'k': return BLACK_KING;
        //DEFAULT
        default: return EMPTY;
    }
}

//sets up starting position
void setupStartingPosition(){
    initBoard(); // Clear the board first
    
    int row = 0, col = 0;
    
    for(char c : startingPosition) {
        if(c == '/') {
            row++;      // Move to next rank
            col = 0;    // Reset to file 'a'
        }
        else if(isdigit(c)) {
            col += (c - '0');  // Skip empty squares (number tells us how many)
        }
        else {
            board[row][col] = charToPiece(c);  // Place the piece
            col++;
        }
    }
}

//prints the board
void printBoard(){
    cout << "\n  +---+---+---+---+---+---+---+---+\n";
    
    for(int row = 0; row < 8; row++) {
        // Print rank number (8, 7, 6, ..., 1)
        cout << (8 - row) << " ";
        
        for(int col = 0; col < 8; col++) {
            cout << "| " << pieceToChar(board[row][col]) << " ";
        }
        cout << "|\n";
        cout << "  +---+---+---+---+---+---+---+---+\n";
    }
    
    // Print file letters
    cout << "    a   b   c   d   e   f   g   h\n\n";
}