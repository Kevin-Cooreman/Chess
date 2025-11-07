#include "game.hpp"
#include "board.hpp"
#include "evaluation.hpp"
#include <iostream>

using namespace std;

int main() {
    cout << "=== BOARD STATE TEST ===" << endl;
    
    ChessGame game;
    Evaluation eval;
    
    // Check initial position
    cout << "After game creation:" << endl;
    cout << "Global board[0][0] = " << board[0][0] << " (should be 0b0010 for white rook)" << endl;
    cout << "Global board[7][0] = " << board[7][0] << " (should be 0b1010 for black rook)" << endl;
    
    // Count material manually from global board
    int pieceCount = 0;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (board[r][c] != EMPTY) pieceCount++;
        }
    }
    cout << "Pieces on global board: " << pieceCount << " (should be 32)" << endl;
    
    // Now check what evaluation sees
    double material = eval.materialCount(game);
    cout << "Evaluation materialCount: " << material << " (should be 0.0 for equal material)" << endl;
    
    // Make a move
    cout << "\nAfter e2e4:" << endl;
    game.makePlayerMove("e2e4");
    
    cout << "Global board[6][4] = " << board[6][4] << " (should be EMPTY)" << endl;
    cout << "Global board[4][4] = " << board[4][4] << " (should be 0b0001 for white pawn)" << endl;
    
    pieceCount = 0;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (board[r][c] != EMPTY) pieceCount++;
        }
    }
    cout << "Pieces on global board: " << pieceCount << " (should still be 32)" << endl;
    
    material = eval.materialCount(game);
    cout << "Evaluation materialCount: " << material << endl;
    
    // Remove a black piece to create imbalance
    cout << "\nManually removing black knight from board[0][1]:" << endl;
    board[0][1] = EMPTY;
    
    material = eval.materialCount(game);
    cout << "Material after removing black knight: " << material << " (should be +3 for white)" << endl;
    
    return 0;
}
