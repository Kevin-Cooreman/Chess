#include "game.hpp"
#include "engine.hpp"
#include "evaluation.hpp"
#include <iostream>
#include <iomanip>

using namespace std;

int main() {
    cout << "=== TT DEBUG TEST ===" << endl;
    cout << "Search same position twice, check TT reuse" << endl << endl;
    
    ChessGame game;
    Engine engine;
    
    cout << "Position hash: " << hex << game.getZobristHash() << dec << endl << endl;
    
    // First search at depth 3
    cout << "=== FIRST SEARCH (depth 3) ===" << endl;
    Move move1 = engine.getBestMove(game, 3);
    cout << "Nodes: " << engine.nodesSearched << endl;
    cout << "TT hits: " << engine.ttHits << endl;
    cout << "TT hit rate: " << fixed << setprecision(1) 
         << (100.0 * engine.ttHits / engine.nodesSearched) << "%" << endl;
    
    cout << "\n=== SECOND SEARCH (depth 3) ===" << endl;
    Move move2 = engine.getBestMove(game, 3);
    cout << "Nodes: " << engine.nodesSearched << endl;
    cout << "TT hits: " << engine.ttHits << endl;
    cout << "TT hit rate: " << fixed << setprecision(1) 
         << (100.0 * engine.ttHits / engine.nodesSearched) << "%" << endl;
    
    if (engine.ttHits < engine.nodesSearched * 0.9) {
        cout << "\n*** ERROR: Second search should reuse TT and be very fast! ***" << endl;
        cout << "Expected TT hit rate > 90%, got " 
             << fixed << setprecision(1) 
             << (100.0 * engine.ttHits / engine.nodesSearched) << "%" << endl;
    } else {
        cout << "\nSUCCESS: Second search reused TT!" << endl;
    }
    
    return 0;
}
