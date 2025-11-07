#include "game.hpp"
#include "evaluation.hpp"
#include <iostream>

using namespace std;

int main() {
    cout << "=== ZOBRIST HASH DURING SEARCH TEST ===" << endl;
    
    ChessGame game;
    Evaluation eval;
    
    uint64_t hash1 = game.getZobristHash();
    uint64_t computed1 = game.computeZobristHash();
    
    cout << "Initial position:" << endl;
    cout << "Incremental hash: " << hex << hash1 << dec << endl;
    cout << "Computed hash:    " << hex << computed1 << dec << endl;
    
    if (hash1 != computed1) {
        cout << "*** ERROR: Hashes don't match at start! ***" << endl;
        return 1;
    }
    
    // Make a move using engine method
    vector<Move> moves = game.getLegalMoves();
    Move firstMove = moves[0];
    
    cout << "\nMaking move (engine style): " << game.moveToString(firstMove) << endl;
    game.makeMoveForEngine(firstMove);
    
    uint64_t hash2 = game.getZobristHash();
    uint64_t computed2 = game.computeZobristHash();
    
    cout << "After move:" << endl;
    cout << "Incremental hash: " << hex << hash2 << dec << endl;
    cout << "Computed hash:    " << hex << computed2 << dec << endl;
    
    if (hash2 != computed2) {
        cout << "*** ERROR: Hashes don't match after move! ***" << endl;
        cout << "Difference: " << hex << (hash2 ^ computed2) << dec << endl;
        return 1;
    }
    
    // Undo the move
    game.undoMove();
    
    uint64_t hash3 = game.getZobristHash();
    uint64_t computed3 = game.computeZobristHash();
    
    cout << "\nAfter undo:" << endl;
    cout << "Incremental hash: " << hex << hash3 << dec << endl;
    cout << "Computed hash:    " << hex << computed3 << dec << endl;
    
    if (hash3 != computed3) {
        cout << "*** ERROR: Hashes don't match after undo! ***" << endl;
        cout << "Difference: " << hex << (hash3 ^ computed3) << dec << endl;
        return 1;
    }
    
    if (hash3 != hash1) {
        cout << "*** ERROR: Hash not restored to original! ***" << endl;
        return 1;
    }
    
    // Try a few moves in sequence
    cout << "\n=== Testing move sequence ===" << endl;
    for (int i = 0; i < 3; i++) {
        vector<Move> legalMoves = game.getLegalMoves();
        Move move = legalMoves[i % legalMoves.size()];
        
        game.makeMoveForEngine(move);
        
        uint64_t inc = game.getZobristHash();
        uint64_t comp = game.computeZobristHash();
        
        cout << "Move " << (i+1) << " " << game.moveToString(move) << ": ";
        if (inc == comp) {
            cout << "OK" << endl;
        } else {
            cout << "MISMATCH!" << endl;
            cout << "  Incremental: " << hex << inc << dec << endl;
            cout << "  Computed:    " << hex << comp << dec << endl;
            cout << "  Difference:  " << hex << (inc ^ comp) << dec << endl;
            return 1;
        }
    }
    
    cout << "\n=== SUCCESS: All hashes match! ===" << endl;
    return 0;
}
