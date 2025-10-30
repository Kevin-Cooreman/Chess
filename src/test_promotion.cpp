#include <iostream>
#include "board.hpp"
#include "game.hpp"

using namespace std;

int main() {
    cout << "Testing pawn promotion...\n";
    
    ChessGame game;
    
    // Set up a position where white pawn can promote
    // Clear the board first
    initBoard();
    
    // Place a white pawn on the 7th rank (row 1) ready to promote
    board[1][4] = WHITE_PAWN;  // e7
    board[7][4] = WHITE_KING;  // White king on e1
    board[0][4] = BLACK_KING;  // Black king on e8
    
    game.displayBoard();
    
    cout << "\nTry to promote the pawn with: e7e8\n";
    cout << "This should ask you to choose promotion piece.\n";
    
    return 0;
}