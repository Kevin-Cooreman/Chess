#include "game.hpp"
#include "engine.hpp"
#include "evaluation.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <random>

using namespace std;

// Configuration for evaluation weights
struct EvalConfig {
    string name;
    double materialWeight;
    double positionWeight;
    double kingSafetyWeight;
    double pawnStructureWeight;
};

// Custom evaluation class that allows weight adjustment
class TunableEvaluation : public Evaluation {
private:
    double materialWeight;
    double positionWeight;
    double kingSafetyWeight;
    double pawnStructureWeight;

public:
    TunableEvaluation(double mw, double pw, double ksw, double psw) 
        : materialWeight(mw), positionWeight(pw), kingSafetyWeight(ksw), pawnStructureWeight(psw) {}

    double evaluate(const ChessGame& game) const {
        double evaluation = 0.0;

        evaluation += materialWeight * materialCount(game);
        evaluation += positionWeight * position(game);
        evaluation += kingSafetyWeight * kingsafety(game);
        evaluation += pawnStructureWeight * pawnStructure(game);

        return evaluation;
    }
};

// Play one game between two configurations
string playGame(EvalConfig& config1, EvalConfig& config2, bool config1PlaysWhite, int depth, int maxMoves = 200) {
    ChessGame game;
    
    TunableEvaluation eval1(config1.materialWeight, config1.positionWeight, 
                            config1.kingSafetyWeight, config1.pawnStructureWeight);
    TunableEvaluation eval2(config2.materialWeight, config2.positionWeight, 
                            config2.kingSafetyWeight, config2.pawnStructureWeight);
    
    Engine engine1(eval1);
    Engine engine2(eval2);
    
    // Play 2-5 random opening moves to create more variety
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<> openingMovesDist(2, 5);
    int randomOpeningMoves = openingMovesDist(gen);
    
    for (int i = 0; i < randomOpeningMoves && !game.isGameOver(); i++) {
        vector<Move> legalMoves = game.getLegalMoves();
        if (legalMoves.empty()) break;
        
        // Pick a random legal move
        uniform_int_distribution<> moveDist(0, legalMoves.size() - 1);
        Move randomMove = legalMoves[moveDist(gen)];
        game.makeEngineMove(randomMove);
    }
    
    int moveCount = 0;
    
    while (!game.isGameOver() && moveCount < maxMoves) {
        bool isWhiteTurn = game.isWhiteToMove();
        
        // Determine which engine plays
        Engine* currentEngine = nullptr;
        if ((isWhiteTurn && config1PlaysWhite) || (!isWhiteTurn && !config1PlaysWhite)) {
            currentEngine = &engine1;
        } else {
            currentEngine = &engine2;
        }
        
        // Get and make move
        Move bestMove = currentEngine->getBestMove(game, depth);
        
        if (bestMove.startRow == -1) {
            break; // No legal moves
        }
        
        game.makeEngineMove(bestMove);
        moveCount++;
    }
    
    // Determine result
    if (game.isGameOver()) {
        string result = game.getGameResult();
        
        if (result.find("White wins") != string::npos) {
            return config1PlaysWhite ? "config1" : "config2";
        } else if (result.find("Black wins") != string::npos) {
            return config1PlaysWhite ? "config2" : "config1";
        } else {
            return "draw";
        }
    }
    
    // If max moves reached, consider it a draw
    return "draw";
}

// Run a match between two configurations
void runMatch(EvalConfig& config1, EvalConfig& config2, int gamesPerSide, int depth) {
    cout << "\n===========================================\n";
    cout << "Match: " << config1.name << " vs " << config2.name << "\n";
    cout << "Games per side: " << gamesPerSide << ", Depth: " << depth << "\n";
    cout << "===========================================\n";
    
    int config1Wins = 0;
    int config2Wins = 0;
    int draws = 0;
    
    // Play games with config1 as white
    cout << "\nPlaying games with " << config1.name << " as White...\n";
    for (int i = 0; i < gamesPerSide; i++) {
        cout << "Game " << (i + 1) << "/" << gamesPerSide << "... ";
        cout.flush();
        
        string result = playGame(config1, config2, true, depth);
        
        if (result == "config1") {
            config1Wins++;
            cout << config1.name << " wins!\n";
        } else if (result == "config2") {
            config2Wins++;
            cout << config2.name << " wins!\n";
        } else {
            draws++;
            cout << "Draw!\n";
        }
    }
    
    // Play games with config1 as black
    cout << "\nPlaying games with " << config1.name << " as Black...\n";
    for (int i = 0; i < gamesPerSide; i++) {
        cout << "Game " << (i + 1) << "/" << gamesPerSide << "... ";
        cout.flush();
        
        string result = playGame(config1, config2, false, depth);
        
        if (result == "config1") {
            config1Wins++;
            cout << config1.name << " wins!\n";
        } else if (result == "config2") {
            config2Wins++;
            cout << config2.name << " wins!\n";
        } else {
            draws++;
            cout << "Draw!\n";
        }
    }
    
    // Display results
    cout << "\n===========================================\n";
    cout << "Final Results:\n";
    cout << config1.name << ": " << config1Wins << " wins\n";
    cout << config2.name << ": " << config2Wins << " wins\n";
    cout << "Draws: " << draws << "\n";
    cout << "Total games: " << (gamesPerSide * 2) << "\n";
    
    double config1Score = config1Wins + (draws * 0.5);
    double config2Score = config2Wins + (draws * 0.5);
    double totalGames = gamesPerSide * 2;
    
    cout << "\nScores (1 point per win, 0.5 per draw):\n";
    cout << config1.name << ": " << config1Score << "/" << totalGames 
         << " (" << (config1Score / totalGames * 100) << "%)\n";
    cout << config2.name << ": " << config2Score << "/" << totalGames 
         << " (" << (config2Score / totalGames * 100) << "%)\n";
    cout << "===========================================\n\n";
}

int main() {
    cout << "Chess Engine Tuning - Self-Play Testing\n";
    cout << "========================================\n\n";
    
    // Define configurations to test - EXTREME DIFFERENCES
    vector<EvalConfig> configs = {
        {"Balanced", 10.0, 3.0, 3.0, 0.05},
        {"MaterialObsessed", 50.0, 1.0, 1.0, 0.01},    // Cares almost only about pieces
        {"PositionalMaster", 8.0, 12.0, 2.0, 0.1},     // Loves mobility and activity
        {"Defensive", 10.0, 2.0, 10.0, 0.05},          // Paranoid about king safety
        {"Aggressive", 8.0, 8.0, 0.5, 0.05},           // Ignores king, seeks activity
    };
    
    cout << "Testing configurations:\n";
    for (size_t i = 0; i < configs.size(); i++) {
        cout << i + 1 << ". " << configs[i].name << ": "
             << "M=" << configs[i].materialWeight 
             << ", P=" << configs[i].positionWeight
             << ", KS=" << configs[i].kingSafetyWeight
             << ", PS=" << configs[i].pawnStructureWeight << "\n";
    }
    
    cout << "\nEnter number of games per side (recommended 5-10): ";
    int gamesPerSide;
    cin >> gamesPerSide;
    
    cout << "Enter search depth (recommended 2-3 for speed): ";
    int depth;
    cin >> depth;
    
    cout << "\nStarting tournament...\n";
    
    // Run round-robin tournament
    for (size_t i = 0; i < configs.size(); i++) {
        for (size_t j = i + 1; j < configs.size(); j++) {
            runMatch(configs[i], configs[j], gamesPerSide, depth);
        }
    }
    
    cout << "Tournament complete!\n";
    cout << "\nBased on the results, the configuration with the highest win rate\n";
    cout << "is likely the best. You can then test variations of the winner.\n";
    
    return 0;
}
