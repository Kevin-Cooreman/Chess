#include "game.hpp"
#include "engine.hpp"
#include "evaluation.hpp"
#include <iostream>
#include <iomanip>

using namespace std;

int main() {
    cout << "=== ENGINE SELF-PLAY TEST ===" << endl;
    cout << "Engine plays against itself for 10 moves" << endl << endl;
    
    ChessGame game;
    Evaluation eval;
    Engine engine(eval);
    
    for (int i = 0; i < 10; i++) {
        cout << "Move " << (i+1) << " (" << (game.isWhiteToMove() ? "White" : "Black") << "): ";
        cout.flush();
        
        uint64_t hashBefore = game.getZobristHash();
        double evalBefore = eval.evaluate(game);
        
        Move best = engine.getBestMove(game, 5);
        
        if (best.startRow == -1) {
            cout << "No legal moves!" << endl;
            break;
        }
        
        cout << game.moveToString(best);
        
        game.makeEngineMove(best);
        
        uint64_t hashAfter = game.getZobristHash();
        double evalAfter = eval.evaluate(game);
        
        cout << " | Eval: " << fixed << setprecision(2) << evalBefore << " -> " << evalAfter;
        cout << " | Nodes: " << engine.nodesSearched;
        cout << " | TT hits: " << engine.ttHits;
        cout << endl;
        
        // Verify hash changed
        if (hashAfter == hashBefore) {
            cout << "*** ERROR: Hash didn't change after move! ***" << endl;
            return 1;
        }
        
        // Verify hash is correct
        uint64_t computed = game.computeZobristHash();
        if (hashAfter != computed) {
            cout << "*** ERROR: Hash mismatch! ***" << endl;
            cout << "Incremental: " << hex << hashAfter << dec << endl;
            cout << "Computed:    " << hex << computed << dec << endl;
            return 1;
        }
    }
    
    cout << "\n=== SUCCESS: 10 moves played successfully! ===" << endl;
    return 0;
}
