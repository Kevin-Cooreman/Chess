#include <iostream>
#include "board.hpp"
#include "game.hpp"

using namespace std;

int main() {
    cout << "Creating game...\n";
    ChessGame game;
    cout << "Game created.\n";
    
    cout << "Testing displayBoard...\n";
    game.displayBoard();
    cout << "displayBoard completed.\n";
    
    cout << "Testing isInCheck...\n";
    bool inCheck = game.isInCheck();
    cout << "isInCheck result: " << inCheck << "\n";
    
    cout << "Testing getLegalMoves...\n";
    auto moves = game.getLegalMoves();
    cout << "Found " << moves.size() << " legal moves.\n";
    
    cout << "All tests completed successfully!\n";
    return 0;
}