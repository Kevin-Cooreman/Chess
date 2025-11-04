// compare_versions.cpp - Test new evaluation versions against v1 baseline
#include "game.hpp"
#include "engine.hpp"
#include "evaluation.hpp"
#include "evaluation_v1.hpp"
#include <iostream>
#include <string>

using namespace std;

// Wrapper to use v1 evaluation with Engine
class EvalV1Wrapper : public Evaluation {
private:
    EvaluationV1 v1;
public:
    double evaluate(const ChessGame& game) const {
        return v1.evaluate(game);
    }
};

string playGame(Evaluation& eval1, Evaluation& eval2, bool eval1PlaysWhite, int depth) {
    ChessGame game;
    Engine engine1(eval1);
    Engine engine2(eval2);
    
    int moveCount = 0;
    int maxMoves = 120;
    
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
    cout << "Evaluation Version Comparison\n";
    cout << "==============================\n";
    cout << "Testing NEW evaluation vs V1 baseline\n\n";
    
    int numGames;
    int depth;
    
    cout << "Number of games to play: ";
    cin >> numGames;
    
    cout << "Search depth: ";
    cin >> depth;
    
    cout << "\nStarting " << numGames << " games at depth " << depth << "...\n\n";
    
    EvalV1Wrapper v1Eval;
    Evaluation newEval;
    
    int newWins = 0;
    int v1Wins = 0;
    int draws = 0;
    
    for (int i = 0; i < numGames; i++) {
        bool newPlaysWhite = (i % 2 == 0);
        
        cout << "Game " << (i + 1) << "/" << numGames << " ";
        cout << "(NEW plays " << (newPlaysWhite ? "White" : "Black") << ")... ";
        cout.flush();
        
        string result = playGame(newEval, v1Eval, newPlaysWhite, depth);
        
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
