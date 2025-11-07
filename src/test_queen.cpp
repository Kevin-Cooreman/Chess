#include "game.hpp"
#include "engine.hpp"
#include "evaluation.hpp"
#include <iostream>

using namespace std;

int main() {
    cout << "=== HANGING QUEEN TEST ===" << endl;
    
    ChessGame game;
    // Place a black knight on c3 so c3xe2 is a legal capture
    game.loadFEN("rnbqkbnr/pppp1ppp/8/4p3/4P3/2n5/PPPPQPPP/RNB1KBNR b KQkq - 0 1");
    
    cout << "Position: White queen on e2 is undefended" << endl;
    cout << "Black to move" << endl << endl;
    
    // Check what's on e2
    cout << "board[6][4] (e2) = " << board[6][4] << " (should be white queen = 5)" << endl;
    
    Evaluation eval;
    cout << "Material count: " << eval.materialCount(game) << " (white has +9 queen advantage)" << endl;
    
    // Get legal moves
    vector<Move> moves = game.getLegalMoves();
    cout << "\nBlack has " << moves.size() << " legal moves" << endl;
    
        // Print board to inspect piece placement
        game.displayBoard();

        // Check pseudo-legal moves (ignore legality) to see if any piece can attack e2
        cout << "\nChecking pseudo-legal moves (not filtering checks):" << endl;
        bool pseudoFound = false;
        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                int piece = board[r][c];
                if (isEmpty(piece) || isWhite(piece)) continue; // only black pieces
                vector<Move> pmoves = generateMovesForPiece(r, c);
                for (const Move &pm : pmoves) {
                    if (pm.targetRow == 6 && pm.targetColumn == 4) {
                        cout << "Pseudo-legal capture from " << char('a'+pm.startColumn) << (8-pm.startRow)
                             << " -> " << game.moveToString(pm) << endl;
                        pseudoFound = true;
                    }
                }
            }
        }
        if (!pseudoFound) cout << "No pseudo-legal captures to e2 found." << endl;
    
    // Check if queen capture is available
    for (const Move& m : moves) {
        if (m.targetRow == 6 && m.targetColumn == 4) {
            cout << "Queen capture available: " << game.moveToString(m) << endl;
        }
    }
    
    // Diagnostic: evaluate each legal move by making it and printing material/eval
    cout << "\nPer-move diagnostic (material, full eval after move):" << endl;
    Evaluation eval2;
    for (size_t i = 0; i < moves.size(); ++i) {
        const Move &m = moves[i];
        int preCaptured = board[m.targetRow][m.targetColumn];
        bool isCapture = !isEmpty(preCaptured) || m.moveType == EN_PASSANT;
        cout << i << ": " << game.moveToString(m) << (isCapture ? " [capture]" : "") << " -> ";
        game.makeMoveForEngine(m);
        cout << "Material: " << eval2.materialCount(game) << ", Eval: " << eval2.evaluate(game) << endl;
        game.undoMove();
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
