#include <iostream>
#include "board.hpp"
#include "moveGeneration.hpp"

using namespace std;

int main() {
    cout << "Testing basic functionality...\n";
    
    // Initialize board
    initBoard();
    setupStartingPosition();
    printBoard();
    
    // Test move generation
    cout << "\nTesting move generation...\n";
    cout << "About to call generateLegalMoves...\n";
    vector<Move> moves = generateLegalMoves(true);
    cout << "Returned from generateLegalMoves...\n";
    cout << "Found " << moves.size() << " legal moves for white.\n";
    
    if (!moves.empty()) {
        cout << "First move: " << moves[0].startRow << "," << moves[0].startColumn 
             << " to " << moves[0].targetRow << "," << moves[0].targetColumn << endl;
    }
    
    return 0;
}