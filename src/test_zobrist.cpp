#include "game.hpp"
#include <iostream>
#include <iomanip>

using namespace std;

int main() {
    cout << "=== ZOBRIST HASH STABILITY TEST ===" << endl;
    cout << "Testing that hash is restored after make/undo moves" << endl << endl;
    
    ChessGame game;
    
    uint64_t originalHash = game.getZobristHash();
    cout << "Original hash: " << hex << originalHash << dec << endl << endl;
    
    vector<Move> legalMoves = game.getLegalMoves();
    
    int errors = 0;
    for (int i = 0; i < min(10, (int)legalMoves.size()); i++) {
        const Move& move = legalMoves[i];
        
        cout << "Testing move " << (i+1) << ": " 
             << (char)('a' + move.startColumn) << (8 - move.startRow)
             << (char)('a' + move.targetColumn) << (8 - move.targetRow) << endl;
        
        game.makeMoveForEngine(move);
        uint64_t hashAfterMove = game.getZobristHash();
        cout << "  After move:  " << hex << hashAfterMove << dec << endl;
        
        game.undoMove();
        uint64_t hashAfterUndo = game.getZobristHash();
        cout << "  After undo:  " << hex << hashAfterUndo << dec << endl;
        
        if (hashAfterUndo != originalHash) {
            cout << "  *** ERROR: Hash not restored! ***" << endl;
            cout << "  Difference:  " << hex << (hashAfterUndo ^ originalHash) << dec << endl;
            errors++;
        } else {
            cout << "  OK" << endl;
        }
        cout << endl;
    }
    
    cout << "=== RESULTS ===" << endl;
    if (errors == 0) {
        cout << "SUCCESS: All " << min(10, (int)legalMoves.size()) << " moves tested, hash stable!" << endl;
    } else {
        cout << "FAILURE: " << errors << " errors found" << endl;
    }
    
    return errors > 0 ? 1 : 0;
}
