#include "game.hpp"
#include "evaluation.hpp"
#include <iostream>
#include <iomanip>

using namespace std;

int main() {
    cout << "=== FULL EVALUATION TEST ===" << endl;
    
    ChessGame game;
    Evaluation eval;
    
    // Test starting position
    cout << "Starting position:" << endl;
    double startEval = eval.evaluate(game);
    cout << "Total evaluation: " << fixed << setprecision(2) << startEval << endl;
    
    // Test a few moves
    game.makePlayerMove("e2e4");
    double e4Eval = eval.evaluate(game);
    cout << "\nAfter e4:" << endl;
    cout << "Total evaluation: " << e4Eval << endl;
    
    game.makePlayerMove("e7e5");
    double e5Eval = eval.evaluate(game);
    cout << "\nAfter e4 e5:" << endl;
    cout << "Total evaluation: " << e5Eval << endl;
    
    game.makePlayerMove("g1f3");
    double nf3Eval = eval.evaluate(game);
    cout << "\nAfter e4 e5 Nf3:" << endl;
    cout << "Total evaluation: " << nf3Eval << endl;
    
    // Test a position where white is up material
    cout << "\n\n=== POSITION WITH MATERIAL IMBALANCE ===" << endl;
    ChessGame game2;
    game2.loadFEN("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");  // Equal
    cout << "Equal position:" << endl;
    cout << "Evaluation: " << eval.evaluate(game2) << endl;
    
    // Remove black's queen
    ChessGame game3;
    game3.loadFEN("rnb1kbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");  // Black missing queen
    cout << "\nBlack missing queen:" << endl;
    double missingQueen = eval.evaluate(game3);
    cout << "Evaluation: " << missingQueen << " (should be very positive for white, around +100 or more)" << endl;
    
    if (missingQueen < 5.0) {
        cout << "\n*** ERROR: Evaluation not seeing material difference! ***" << endl;
        return 1;
    }
    
    cout << "\n=== SUCCESS: Evaluation is working! ===" << endl;
    return 0;
}
