#include "game.hpp"
#include "evaluation.hpp"
#include <iostream>

using namespace std;

int main() {
    ChessGame game;
    // Use position where the white queen on e2 is hanging (black to move)
    // Add a black knight on c3 so the queen on e2 can be captured
    game.loadFEN("rnbqkbnr/pppp1ppp/8/4p3/4P3/2n5/PPPPQPPP/RNB1KBNR b KQkq - 0 1");
    
    Evaluation eval;
    
    cout << "Starting position:" << endl;
    cout << "Material: " << eval.materialCount(game) << endl;
    cout << "Full eval: " << eval.evaluate(game) << endl << endl;
    
    // Find and make the queen capture
    vector<Move> moves = game.getLegalMoves();
    const Move* queenCapture = nullptr;

    cout << "Legal moves (" << moves.size() << "):\n";
    for (size_t i = 0; i < moves.size(); ++i) {
        const Move& m = moves[i];
        cout << i << ": " << game.moveToString(m);
        if (m.targetRow == 6 && m.targetColumn == 4) cout << "  <-- targets e2";
        cout << "\n";
    }

    for (const Move& m : moves) {
        if (m.targetRow == 6 && m.targetColumn == 4) {
            queenCapture = &m;
            cout << "Found queen capture: " << game.moveToString(m) << endl;
            break;
        }
    }

    if (!queenCapture) {
        cout << "ERROR: Queen capture not found!" << endl;
        return 1;
    }

    game.makeMoveForEngine(*queenCapture);
    cout << "\nAfter capturing queen:" << endl;
    cout << "Material: " << eval.materialCount(game) << endl;
    cout << "Full eval: " << eval.evaluate(game) << endl;
    
    cout << "\n(Material should be -9 for black since black captured white's queen)" << endl;
    
    return 0;
}
