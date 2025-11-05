// compare_versions.cpp - Test new versions against V1 baselines
#include "game.hpp"
#include "engine.hpp"
#include "engine_v1.hpp"
#include "evaluation.hpp"
#include "evaluation_v1.hpp"
#include <iostream>
#include <string>

using namespace std;

// Wrapper to use v1 evaluation with current Engine
class EvalV1Wrapper : public Evaluation {
private:
    EvaluationV1 v1;
public:
    double evaluate(const ChessGame& game) const {
        return v1.evaluate(game);
    }
};

// Varied starting positions for testing
vector<string> testPositions = {
    // Initial position
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    
    // Open middlegame positions
    "r1bqkb1r/pppp1ppp/2n2n2/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4", // Italian Game
    "rnbqkb1r/ppp2ppp/4pn2/3p4/2PP4/2N2N2/PP2PPPP/R1BQKB1R w KQkq - 0 5", // Queen's Gambit
    "r1bqk2r/pppp1ppp/2n2n2/2b1p3/2B1P3/3P1N2/PPP2PPP/RNBQK2R w KQkq - 4 5", // Giuoco Piano
    
    // Tactical middlegame
    "r3kb1r/pp1nqppp/2p1pn2/3p1b2/2PP4/2NBPN2/PP3PPP/R1BQK2R w KQkq - 2 8",
    "r1bqk2r/pp2bppp/2nppn2/8/3NP3/2N1B3/PPPQ1PPP/R3KB1R w KQkq - 2 9",
    
    // Endgame positions
    "8/5pk1/6p1/3K4/8/8/5PPP/8 w - - 0 1", // King and pawn endgame
    "4k3/8/8/8/8/4K3/4P3/8 w - - 0 1", // Simple pawn endgame
    "6k1/5ppp/8/8/8/8/5PPP/6K1 w - - 0 1", // Symmetric pawn endgame
    
    // Positions with imbalances
    "rnbqkb1r/pp3ppp/2p1pn2/3p4/2PP4/5NP1/PP2PP1P/RNBQKB1R w KQkq - 0 6", // King's Indian structure
};

string playGame(Evaluation& eval1, Evaluation& eval2, bool eval1PlaysWhite, int depth, const string& startingFEN) {
    ChessGame game;
    if (!startingFEN.empty()) {
        game.loadFEN(startingFEN);
    }
    
    Engine engine1(eval1);
    Engine engine2(eval2);
    
    int moveCount = 0;
    int maxMoves = 80;  // Reduced from 120 to get more decisive games
    
    while (!game.isGameOver() && moveCount < maxMoves) {
        bool isWhiteTurn = game.isWhiteToMove();
        
        Engine* currentEngine = nullptr;
        if ((isWhiteTurn && eval1PlaysWhite) || (!isWhiteTurn && !eval1PlaysWhite)) {
            currentEngine = &engine1;
        } else {
            currentEngine = &engine2;
        }
        
        Move bestMove = currentEngine->getBestMove(game, depth);
        
        if (bestMove.startRow == -1) {
            break;
        }
        
        game.makeEngineMove(bestMove);
        moveCount++;
    }
    
    if (game.isGameOver()) {
        string result = game.getGameResult();
        if (result.find("White wins") != string::npos) {
            return eval1PlaysWhite ? "new" : "v1";
        } else if (result.find("Black wins") != string::npos) {
            return eval1PlaysWhite ? "v1" : "new";
        }
    }
    
    return "draw";
}

int main() {
    cout << "Version Comparison Tool\n";
    cout << "==============================\n";
    cout << "1. Compare NEW Evaluation vs V1 Evaluation (same engine)\n";
    cout << "2. Compare NEW Engine vs V1 Engine (same evaluation)\n";
    cout << "3. Compare FULL NEW (engine+eval) vs FULL V1 (engine+eval)\n";
    cout << "\nSelect comparison type (1-3): ";
    
    int comparisonType;
    cin >> comparisonType;
    
    int numGames;
    int depth;
    
    cout << "Number of games to play: ";
    cin >> numGames;
    
    cout << "Search depth: ";
    cin >> depth;
    
    cout << "\n==============================\n";
    
    int newWins = 0;
    int v1Wins = 0;
    int draws = 0;
    
    if (comparisonType == 1) {
        // Compare evaluations using current engine
        cout << "Testing: NEW Evaluation vs V1 Evaluation\n";
        cout << "(Both using current Engine)\n\n";
        
        EvalV1Wrapper v1Eval;
        Evaluation newEval;
        
        for (int i = 0; i < numGames; i++) {
            bool newPlaysWhite = (i % 2 == 0);
            string startingFEN = testPositions[i % testPositions.size()];
            
            cout << "Game " << (i + 1) << "/" << numGames << " ";
            cout << "(NEW plays " << (newPlaysWhite ? "White" : "Black") << ")... ";
            cout.flush();
            
            string result = playGame(newEval, v1Eval, newPlaysWhite, depth, startingFEN);
            
            if (result == "new") {
                newWins++;
                cout << "NEW wins!\n";
            } else if (result == "v1") {
                v1Wins++;
                cout << "V1 wins\n";
            } else {
                draws++;
                cout << "Draw\n";
            }
        }
    } 
    else if (comparisonType == 2) {
        // Compare engines using current evaluation
        cout << "Testing: NEW Engine vs V1 Engine\n";
        cout << "(Both using current Evaluation)\n";
        cout << "(V1 Engine locked to depth 4 - maximum manageable)\n\n";
        
        Evaluation eval;
        
        for (int i = 0; i < numGames; i++) {
            bool newPlaysWhite = (i % 2 == 0);
            string startingFEN = testPositions[i % testPositions.size()];
            
            ChessGame game;
            if (!startingFEN.empty()) {
                game.loadFEN(startingFEN);
            }
            
            Engine newEngine(eval);
            EngineV1 v1Engine(eval);
            
            int moveCount = 0;
            int maxMoves = 80;
            
            cout << "Game " << (i + 1) << "/" << numGames << " ";
            cout << "(NEW plays " << (newPlaysWhite ? "White" : "Black") << ")... ";
            cout.flush();
            
            while (!game.isGameOver() && moveCount < maxMoves) {
                bool isWhiteTurn = game.isWhiteToMove();
                Move bestMove(-1, -1, -1, -1);
                
                if ((isWhiteTurn && newPlaysWhite) || (!isWhiteTurn && !newPlaysWhite)) {
                    bestMove = newEngine.getBestMove(game, depth);
                } else {
                    bestMove = v1Engine.getBestMove(game, 4);  // V1 always at depth 4
                }
                
                if (bestMove.startRow == -1) break;
                
                game.makeEngineMove(bestMove);
                moveCount++;
            }
            
            string result = "draw";
            if (game.isGameOver()) {
                string gameResult = game.getGameResult();
                if (gameResult.find("White wins") != string::npos) {
                    result = newPlaysWhite ? "new" : "v1";
                } else if (gameResult.find("Black wins") != string::npos) {
                    result = newPlaysWhite ? "v1" : "new";
                }
            }
            
            if (result == "new") {
                newWins++;
                cout << "NEW wins!\n";
            } else if (result == "v1") {
                v1Wins++;
                cout << "V1 wins\n";
            } else {
                draws++;
                cout << "Draw\n";
            }
        }
    }
    else if (comparisonType == 3) {
        // Compare full versions (engine + evaluation)
        cout << "Testing: FULL NEW vs FULL V1\n";
        cout << "(NEW: Current Engine + Current Evaluation)\n";
        cout << "(V1: V1 Engine + V1 Evaluation)\n";
        cout << "(V1 Engine locked to depth 4 - maximum manageable)\n\n";
        
        Evaluation newEval;
        EvalV1Wrapper v1Eval;
        
        for (int i = 0; i < numGames; i++) {
            bool newPlaysWhite = (i % 2 == 0);
            string startingFEN = testPositions[i % testPositions.size()];
            
            ChessGame game;
            if (!startingFEN.empty()) {
                game.loadFEN(startingFEN);
            }
            
            Engine newEngine(newEval);
            EngineV1 v1Engine(v1Eval);
            
            int moveCount = 0;
            int maxMoves = 80;
            
            cout << "Game " << (i + 1) << "/" << numGames << " ";
            cout << "(NEW plays " << (newPlaysWhite ? "White" : "Black") << ")... ";
            cout.flush();
            
            while (!game.isGameOver() && moveCount < maxMoves) {
                bool isWhiteTurn = game.isWhiteToMove();
                Move bestMove(-1, -1, -1, -1);
                
                if ((isWhiteTurn && newPlaysWhite) || (!isWhiteTurn && !newPlaysWhite)) {
                    bestMove = newEngine.getBestMove(game, depth);
                } else {
                    bestMove = v1Engine.getBestMove(game, 4);  // V1 always at depth 4
                }
                
                if (bestMove.startRow == -1) break;
                
                game.makeEngineMove(bestMove);
                moveCount++;
            }
            
            string result = "draw";
            if (game.isGameOver()) {
                string gameResult = game.getGameResult();
                if (gameResult.find("White wins") != string::npos) {
                    result = newPlaysWhite ? "new" : "v1";
                } else if (gameResult.find("Black wins") != string::npos) {
                    result = newPlaysWhite ? "v1" : "new";
                }
            }
            
            if (result == "new") {
                newWins++;
                cout << "NEW wins!\n";
            } else if (result == "v1") {
                v1Wins++;
                cout << "V1 wins\n";
            } else {
                draws++;
                cout << "Draw\n";
            }
        }
    }
    else {
        cout << "Invalid selection!\n";
        return 1;
    }
    
    cout << "\n==============================\n";
    cout << "RESULTS:\n";
    cout << "==============================\n";
    cout << "NEW version: " << newWins << " wins\n";
    cout << "V1 baseline: " << v1Wins << " wins\n";
    cout << "Draws:       " << draws << "\n\n";
    
    double newWinRate = (newWins + 0.5 * draws) / numGames * 100;
    cout << "NEW win rate: " << newWinRate << "%\n";
    
    if (newWins > v1Wins) {
        cout << "\n✓ NEW version is BETTER! (+";
        cout << (newWins - v1Wins) << ")\n";
    } else if (v1Wins > newWins) {
        cout << "\n✗ V1 baseline is still better (-";
        cout << (v1Wins - newWins) << ")\n";
    } else {
        cout << "\n= Versions are EQUAL\n";
    }
    
    return 0;
}
