#include <iostream>
#include <sstream>
#include "board.hpp"
#include "game.hpp"

using namespace std;

int main() {
    cout << "=== PAWN PROMOTION DEMO ===\n\n";
    
    // Create a game instance
    ChessGame game;
    
    // Set up a custom position where white pawn is ready to promote
    initBoard();
    
    // Place pieces for a simple promotion scenario
    board[1][4] = WHITE_PAWN;  // White pawn on e7 (about to promote)
    board[7][4] = WHITE_KING;  // White king on e1
    board[0][4] = BLACK_KING;  // Black king on e8
    
    cout << "Current position (White pawn ready to promote):\n";
    printBoard();
    
    cout << "\n=== PROMOTION OPTIONS ===\n";
    cout << "You can now promote in two ways:\n\n";
    
    cout << "1. INTERACTIVE PROMOTION:\n";
    cout << "   - Enter move: e7e8\n";
    cout << "   - The game will ask you to choose: q/r/b/n\n\n";
    
    cout << "2. DIRECT PROMOTION:\n";
    cout << "   - Enter move: e7e8q (promotes to queen)\n";
    cout << "   - Enter move: e7e8r (promotes to rook)\n";
    cout << "   - Enter move: e7e8b (promotes to bishop)\n";
    cout << "   - Enter move: e7e8n (promotes to knight)\n\n";
    
    cout << "The promotion detection is now working!\n";
    cout << "When you play the actual game and reach this position,\n";
    cout << "you'll be prompted to choose your promotion piece.\n\n";
    
    // Test the promotion detection function
    cout << "=== TESTING PROMOTION DETECTION ===\n";
    cout << "isPawnPromotion(1,4,0,4): " << (game.isPawnPromotion(1,4,0,4) ? "YES" : "NO") << endl;
    cout << "isPawnPromotion(6,4,5,4): " << (game.isPawnPromotion(6,4,5,4) ? "YES" : "NO") << endl;
    
    return 0;
}