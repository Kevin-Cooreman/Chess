#include "game.hpp"
#include "engine.hpp"
#include "evaluation.hpp"
#include <iostream>

using namespace std;

int main() {
    cout << "=== TACTICAL ABILITY TEST ===" << endl << endl;
    
    Evaluation eval;
    Engine engine(eval);
    int passed = 0;
    int failed = 0;
    
    // Test 1: Capture hanging queen
    cout << "Test 1: Capture hanging queen" << endl;
    ChessGame game1;
    // Make the queen on e2 actually capturable (black knight on c3)
    game1.loadFEN("rnbqkbnr/pppp1ppp/8/4p3/4P3/2n5/PPPPQPPP/RNB1K1NR b KQkq - 0 1");  // Queen on e2, capturable by c3
    // Diagnostic: list legal moves and per-move evals
    vector<Move> moves1 = game1.getLegalMoves();
    cout << "Legal moves (" << moves1.size() << "):\n";
    for (size_t i = 0; i < moves1.size(); ++i) {
        const Move &m = moves1[i];
        cout << i << ": " << game1.moveToString(m);
        if (m.targetRow == 6 && m.targetColumn == 4) cout << "  <-- targets e2";
        cout << "\n";
    }
    cout << "\nPer-move evals (material, eval after move):\n";
    for (const Move &m : moves1) {
        int captured = board[m.targetRow][m.targetColumn];
        bool isCap = !isEmpty(captured) || m.moveType == EN_PASSANT;
        cout << game1.moveToString(m) << (isCap ? " [capture]" : "") << " -> ";
        game1.makeMoveForEngine(m);
        cout << "Material: " << eval.materialCount(game1) << ", Eval: " << eval.evaluate(game1) << "\n";
        game1.undoMove();
    }

    Move move1 = engine.getBestMove(game1, 5);

    if (move1.targetRow == 6 && move1.targetColumn == 4) {
        cout << "✓ PASS - Captures queen on e2" << endl;
        passed++;
    } else {
        cout << "✗ FAIL - Doesn't capture queen: " << game1.moveToString(move1) << endl;
        failed++;
    }
    
    // Test 2: Avoid checkmate in 1
    cout << "\nTest 2: Avoid checkmate in 1" << endl;
    ChessGame game2;
    game2.loadFEN("rnb1kbnr/pppp1ppp/8/4p2q/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1");
    Move move2 = engine.getBestMove(game2, 5);
    
    // White must block or move king, any move that doesn't lose immediately is OK
    game2.makeMoveForEngine(move2);
    bool inCheckmate = game2.isInCheckmate();
    game2.undoMove();
    
    if (!inCheckmate) {
        cout << "✓ PASS - Avoids immediate checkmate with " << game2.moveToString(move2) << endl;
        passed++;
    } else {
        cout << "✗ FAIL - Move leads to checkmate: " << game2.moveToString(move2) << endl;
        failed++;
    }
    
    // Test 3: Deliver checkmate with queen and rook vs lone king
    cout << "\nTest 3: Checkmate with Q+R vs K" << endl;
    ChessGame game3;
    game3.loadFEN("7k/5Q2/6R1/8/8/8/8/K7 w - - 0 1"); // White to move, should checkmate
    
    bool foundMate = false;
    for (int depth = 1; depth <= 6; depth++) {
        Move move3 = engine.getBestMove(game3, depth);
        game3.makeMoveForEngine(move3);
        
        if (game3.isInCheckmate()) {
            cout << "✓ PASS - Delivers checkmate at depth " << depth << " with " 
                 << game3.moveToString(move3) << endl;
            foundMate = true;
            passed++;
            break;
        }
        game3.undoMove();
    }
    
    if (!foundMate) {
        cout << "✗ FAIL - Cannot find checkmate in 6 moves" << endl;
        failed++;
    }
    
    // Test 4: Don't stalemate when winning
    cout << "\nTest 4: Win without stalemating (K+Q vs K)" << endl;
    ChessGame game4;
    game4.loadFEN("7k/8/6K1/8/8/8/8/Q7 w - - 0 1"); // Easy KQ vs K endgame
    
    int moveCount = 0;
    bool won = false;
    bool stalemated = false;
    
    while (moveCount < 50 && !game4.isGameOver()) {
        Move move = engine.getBestMove(game4, 5);
        if (move.startRow == -1) break;
        
        game4.makeEngineMove(move);
        moveCount++;
        
        if (game4.isInCheckmate()) {
            won = true;
            break;
        }
        if (game4.isInStalemate()) {
            stalemated = true;
            break;
        }
    }
    
    if (won) {
        cout << "✓ PASS - Delivers checkmate in " << moveCount << " moves" << endl;
        passed++;
    } else if (stalemated) {
        cout << "✗ FAIL - Stalemates instead of checkmating" << endl;
        failed++;
    } else {
        cout << "✗ FAIL - Cannot checkmate in 50 moves" << endl;
        failed++;
    }
    
    // Summary
    cout << "\n==============================" << endl;
    cout << "RESULTS: " << passed << "/" << (passed + failed) << " tests passed" << endl;
    
    if (failed == 0) {
        cout << "✓ All tactical tests passed!" << endl;
    } else {
        cout << "✗ Some tests failed - engine has tactical weaknesses" << endl;
    }
    
    return failed > 0 ? 1 : 0;
}
