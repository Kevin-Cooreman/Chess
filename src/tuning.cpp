#include "game.hpp"
#include "engine.hpp"
#include "evaluation.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <map>
#include <algorithm>

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

// Different starting positions for testing
struct StartingPosition {
    string name;
    string fen;
};

vector<StartingPosition> getTestPositions() {
    return {
        {"Standard Opening", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"},
        {"Open Middlegame", "r1bqk2r/pp2bppp/2n1pn2/3p4/2PP4/2N1PN2/PP2BPPP/R1BQK2R w KQkq - 0 8"},
        {"Tactical Position", "r2qkb1r/ppp2ppp/2n5/3pPb2/3Pn3/2N2N2/PPP2PPP/R1BQKB1R w KQkq - 0 8"},
        {"Closed Position", "rnbqkb1r/pp2pppp/3p1n2/8/3NP3/2N5/PPP2PPP/R1BQKB1R w KQkq - 0 6"},
        {"Early Endgame", "4k3/8/3K4/8/8/8/4P3/8 w - - 0 1"}
    };
}

// Play one game between two configurations
string playGame(EvalConfig& config1, EvalConfig& config2, bool config1PlaysWhite, int depth, const string& startingFen, int maxMoves = 200) {
    ChessGame game;
    
    // Load starting position if provided
    if (!startingFen.empty()) {
        game.loadFEN(startingFen);
    }
    
    TunableEvaluation eval1(config1.materialWeight, config1.positionWeight, 
                            config1.kingSafetyWeight, config1.pawnStructureWeight);
    TunableEvaluation eval2(config2.materialWeight, config2.positionWeight, 
                            config2.kingSafetyWeight, config2.pawnStructureWeight);
    
    Engine engine1(eval1);
    Engine engine2(eval2);
    
    // Only play random opening moves if starting from standard position
    if (startingFen.empty() || startingFen == "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") {
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

// Structure to track overall tournament results
struct TournamentResults {
    map<string, int> wins;
    map<string, int> losses;
    map<string, int> draws;
    map<string, double> scores;
    map<string, int> gamesPlayed;
    
    // Results per position type
    map<string, map<string, int>> winsByPosition;      // config -> position -> wins
    map<string, map<string, int>> lossesByPosition;    // config -> position -> losses
    map<string, map<string, int>> drawsByPosition;     // config -> position -> draws
};

// Run a match between two configurations
void runMatch(EvalConfig& config1, EvalConfig& config2, int gamesPerSide, int depth, 
              const string& positionName, const string& startingFen, TournamentResults& results) {
    cout << "\n===========================================\n";
    cout << "Position: " << positionName << "\n";
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
        
        string result = playGame(config1, config2, true, depth, startingFen);
        
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
        
        string result = playGame(config1, config2, false, depth, startingFen);
        
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
    cout << "Final Results for " << positionName << ":\n";
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
    
    // Update tournament results
    results.wins[config1.name] += config1Wins;
    results.wins[config2.name] += config2Wins;
    results.losses[config1.name] += config2Wins;
    results.losses[config2.name] += config1Wins;
    results.draws[config1.name] += draws;
    results.draws[config2.name] += draws;
    results.scores[config1.name] += config1Score;
    results.scores[config2.name] += config2Score;
    results.gamesPlayed[config1.name] += totalGames;
    results.gamesPlayed[config2.name] += totalGames;
    
    // Update per-position results
    results.winsByPosition[config1.name][positionName] += config1Wins;
    results.winsByPosition[config2.name][positionName] += config2Wins;
    results.lossesByPosition[config1.name][positionName] += config2Wins;
    results.lossesByPosition[config2.name][positionName] += config1Wins;
    results.drawsByPosition[config1.name][positionName] += draws;
    results.drawsByPosition[config2.name][positionName] += draws;
}

int main() {
    cout << "Chess Engine Tuning - Self-Play Testing\n";
    cout << "========================================\n\n";
    
    // Define configurations to test - EXTREME DIFFERENCES
    vector<EvalConfig> allConfigs = {
        {"Balanced", 10.0, 3.0, 3.0, 0.05},
        {"MaterialObsessed", 50.0, 1.0, 1.0, 0.01},    // Cares almost only about pieces
        {"PositionalMaster", 8.0, 12.0, 2.0, 0.1},     // Loves mobility and activity
        {"Defensive", 10.0, 2.0, 10.0, 0.05},          // Paranoid about king safety
        {"Aggressive", 8.0, 8.0, 0.5, 0.05},           // Ignores king, seeks activity
    };
    
    cout << "Select test mode:\n";
    cout << "1. Quick test (Balanced vs MaterialObsessed only)\n";
    cout << "2. Full tournament (all 5 configurations)\n";
    cout << "Choice (1-2): ";
    int modeChoice;
    cin >> modeChoice;
    
    vector<EvalConfig> configs;
    if (modeChoice == 1) {
        configs.push_back(allConfigs[0]);  // Balanced
        configs.push_back(allConfigs[1]);  // MaterialObsessed
    } else {
        configs = allConfigs;
    }
    
    cout << "\nTesting configurations:\n";
    for (size_t i = 0; i < configs.size(); i++) {
        cout << i + 1 << ". " << configs[i].name << ": "
             << "M=" << configs[i].materialWeight 
             << ", P=" << configs[i].positionWeight
             << ", KS=" << configs[i].kingSafetyWeight
             << ", PS=" << configs[i].pawnStructureWeight << "\n";
    }
    
    // Get test positions
    vector<StartingPosition> allPositions = getTestPositions();
    
    cout << "\nAvailable positions:\n";
    for (size_t i = 0; i < allPositions.size(); i++) {
        cout << i + 1 << ". " << allPositions[i].name << "\n";
    }
    
    cout << "\nSelect position to test (1-" << allPositions.size() 
         << "), or 0 for ALL positions: ";
    int posChoice;
    cin >> posChoice;
    
    vector<StartingPosition> positions;
    if (posChoice == 0) {
        positions = allPositions;
    } else if (posChoice >= 1 && posChoice <= (int)allPositions.size()) {
        positions.push_back(allPositions[posChoice - 1]);
    } else {
        cout << "Invalid choice, using Standard Opening\n";
        positions.push_back(allPositions[0]);
    }
    
    cout << "\nEnter number of games per side per position (recommended 2-3): ";
    int gamesPerSide;
    cin >> gamesPerSide;
    
    cout << "Enter search depth (recommended 2-3 for speed): ";
    int depth;
    cin >> depth;
    
    // Calculate and display estimated time
    int totalMatchups = (configs.size() * (configs.size() - 1)) / 2;  // Round-robin
    int gamesPerMatchup = gamesPerSide * 2;
    int totalGames = totalMatchups * gamesPerMatchup * positions.size();
    
    cout << "\n===========================================\n";
    cout << "Tournament Overview:\n";
    cout << "  Configurations: " << configs.size() << "\n";
    cout << "  Matchups: " << totalMatchups << "\n";
    cout << "  Positions: " << positions.size() << "\n";
    cout << "  Games per matchup: " << gamesPerMatchup << "\n";
    cout << "  TOTAL GAMES: " << totalGames << "\n";
    cout << "  Depth: " << depth << "\n";
    
    // Estimate time (rough: 5s per game at depth 2, 20s at depth 3, 60s at depth 4)
    int secondsPerGame = (depth == 2) ? 5 : (depth == 3) ? 20 : 60;
    int estimatedMinutes = (totalGames * secondsPerGame) / 60;
    cout << "  Estimated time: ~" << estimatedMinutes << " minutes\n";
    cout << "===========================================\n";
    
    cout << "\nPress Enter to start, or Ctrl+C to cancel...";
    cin.ignore();
    cin.get();
    
    cout << "\nStarting tournament...\n";
    
    cout << "\nTesting " << positions.size() << " position(s):\n";
    for (const auto& pos : positions) {
        cout << "  - " << pos.name << "\n";
    }
    cout << "\n";
    
    // Initialize tournament results tracking
    TournamentResults results;
    for (const auto& config : configs) {
        results.wins[config.name] = 0;
        results.losses[config.name] = 0;
        results.draws[config.name] = 0;
        results.scores[config.name] = 0.0;
        results.gamesPlayed[config.name] = 0;
    }
    
    // Run round-robin tournament across all positions
    for (const auto& position : positions) {
        cout << "\n\n*** STARTING TESTS FOR POSITION: " << position.name << " ***\n";
        
        for (size_t i = 0; i < configs.size(); i++) {
            for (size_t j = i + 1; j < configs.size(); j++) {
                runMatch(configs[i], configs[j], gamesPerSide, depth, 
                        position.name, position.fen, results);
            }
        }
    }
    
    cout << "\n\n";
    cout << "###############################################\n";
    cout << "#                                             #\n";
    cout << "#        FINAL TOURNAMENT SUMMARY             #\n";
    cout << "#                                             #\n";
    cout << "###############################################\n\n";
    
    // Create a vector of config names sorted by score
    vector<pair<string, double>> rankings;
    for (const auto& config : configs) {
        double winRate = (results.scores[config.name] / results.gamesPlayed[config.name]) * 100.0;
        rankings.push_back({config.name, winRate});
    }
    
    // Sort by win rate (descending)
    sort(rankings.begin(), rankings.end(), 
         [](const pair<string, double>& a, const pair<string, double>& b) {
             return a.second > b.second;
         });
    
    cout << "RANKINGS (by win rate):\n";
    cout << "===========================================\n";
    for (size_t i = 0; i < rankings.size(); i++) {
        string name = rankings[i].first;
        cout << (i + 1) << ". " << name << "\n";
        cout << "   Win Rate: " << rankings[i].second << "%\n";
        cout << "   Record: " << results.wins[name] << "W - " 
             << results.losses[name] << "L - " << results.draws[name] << "D\n";
        cout << "   Score: " << results.scores[name] << "/" << results.gamesPlayed[name] << "\n";
        cout << "   Games Played: " << results.gamesPlayed[name] << "\n";
        
        // Show breakdown by position type
        cout << "\n   Performance by Position:\n";
        for (const auto& pos : positions) {
            int w = results.winsByPosition[name][pos.name];
            int l = results.lossesByPosition[name][pos.name];
            int d = results.drawsByPosition[name][pos.name];
            int total = w + l + d;
            if (total > 0) {
                double posScore = w + (d * 0.5);
                cout << "     " << pos.name << ": " << w << "-" << l << "-" << d 
                     << " (" << (posScore / total * 100) << "%)\n";
            }
        }
        cout << "\n";
    }
    
    cout << "===========================================\n";
    cout << "\nBEST CONFIGURATION: " << rankings[0].first << "\n";
    cout << "with a win rate of " << rankings[0].second << "%\n\n";
    
    // Show which config is best at each position type
    cout << "BEST BY POSITION TYPE:\n";
    cout << "===========================================\n";
    for (const auto& pos : positions) {
        string bestConfig = "";
        double bestScore = -1;
        
        for (const auto& config : configs) {
            int w = results.winsByPosition[config.name][pos.name];
            int l = results.lossesByPosition[config.name][pos.name];
            int d = results.drawsByPosition[config.name][pos.name];
            int total = w + l + d;
            
            if (total > 0) {
                double posScore = (w + (d * 0.5)) / total;
                if (posScore > bestScore) {
                    bestScore = posScore;
                    bestConfig = config.name;
                }
            }
        }
        
        if (!bestConfig.empty()) {
            cout << pos.name << ": " << bestConfig 
                 << " (" << (bestScore * 100) << "%)\n";
        }
    }
    cout << "===========================================\n\n";
    
    cout << "Recommendation: Use " << rankings[0].first << " as your baseline,\n";
    cout << "or test variations of the top 2-3 configurations.\n";
    
    return 0;
}
