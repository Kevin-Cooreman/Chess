#include "game.hpp"
#include "engine.hpp"
#include "evaluation.hpp"
#include <iostream>

using namespace std;

int main() {
    cout << "=== HANGING QUEEN TEST ===" << endl;
    
    ChessGame game;
    game.loadFEN("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPPQPPP/RNB1KBNR b KQkq - 0 1");
    
    cout << "Position: White queen on e2 is undefended" << endl;
    cout << "Black to move" << endl << endl;
    
    // Check what's on e2
    cout << "board[6][4] (e2) = " << board[6][4] << " (should be white queen = 5)" << endl;
    
    Evaluation eval;
    cout << "Material count: " << eval.materialCount(game) << " (white has +9 queen advantage)" << endl;
    
    // Get legal moves
    vector<Move> moves = game.getLegalMoves();
    cout << "\nBlack has " << moves.size() << " legal moves" << endl;
    
    // Check if queen capture is available
    for (const Move& m : moves) {
        if (m.targetRow == 6 && m.targetColumn == 4) {
            cout << "Queen capture available: " << game.moveToString(m) << endl;
        }
    }
    
    // Now ask engine
    Engine engine(eval);
    cout << "\nEngine thinking..." << endl;
    Move best = engine.getBestMove(game, 5);
    cout << "Engine chose: " << game.moveToString(best) << endl;
    
    if (best.targetRow == 6 && best.targetColumn == 4) {
        cout << "SUCCESS: Engine captures the queen!" << endl;
    } else {
        cout << "FAILURE: Engine doesn't capture the queen!" << endl;
        
        // Make the move and see eval
        game.makeMoveForEngine(best);
        cout << "Material after engine move: " << eval.materialCount(game) << endl;
        game.undoMove();
        
        // Make queen capture and see eval
        for (const Move& m : moves) {
            if (m.targetRow == 6 && m.targetColumn == 4) {
                game.makeMoveForEngine(m);
                cout << "Material after capturing queen: " << eval.materialCount(game) << endl;
                break;
            }
        }
    }
    
    return 0;
}
