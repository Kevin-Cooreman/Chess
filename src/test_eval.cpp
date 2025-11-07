#include "game.hpp"
#include "engine.hpp"
#include "evaluation.hpp"
#include <iostream>

using namespace std;

int main() {
    cout << "=== EVALUATION TEST ===" << endl;
    cout << "Testing if NEW engine makes sensible moves" << endl << endl;
    
    ChessGame game;
    Evaluation eval;
    Engine engine(eval);
    
    // Test 1: Starting position - should not blunder material
    cout << "Test 1: Opening moves" << endl;
    for (int i = 0; i < 5; i++) {
        Move best = engine.getBestMove(game, 5);
        cout << "Move " << (i+1) << ": " << game.moveToString(best) << endl;
        game.makeEngineMove(best);
    }
    
    cout << "\nMaterial count after 5 moves: " << eval.materialCount(game) << endl;
    
    // Test 2: Simple tactic - can it capture a hanging piece?
    cout << "\n\nTest 2: Hanging queen" << endl;
    ChessGame game2;
    game2.loadFEN("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPPQPPP/RNB1KBNR b KQkq - 0 1");  // White queen on e2 is hanging
    
    Move blackMove = engine.getBestMove(game2, 5);
    cout << "Black should capture queen: " << game2.moveToString(blackMove) << endl;
    
    // Check if it captures the queen
    if (blackMove.targetRow == 6 && blackMove.targetColumn == 4) {
        cout << "SUCCESS: Engine captures hanging queen!" << endl;
    } else {
        cout << "FAILURE: Engine doesn't capture hanging queen!" << endl;
        cout << "Material eval before: " << eval.materialCount(game2) << endl;
        game2.makeEngineMove(blackMove);
        cout << "Material eval after: " << eval.materialCount(game2) << endl;
    }
    
    return 0;
}
