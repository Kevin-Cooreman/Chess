#include "game.hpp"
#include "engine.hpp"
#include "evaluation.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>

using namespace std;
using namespace chrono;

struct BenchResult {
    string position;
    int depth;
    int nodes;
    double timeMs;
    double nodesPerSec;
    string bestMove;
};

BenchResult benchmarkPosition(ChessGame& game, const string& posName, int depth) {
    Engine engine;
    
    cout << "Testing: " << posName << " (depth " << depth << ")..." << flush;
    
    auto start = high_resolution_clock::now();
    Move bestMove = engine.getBestMove(game, depth);
    auto end = high_resolution_clock::now();
    
    double timeMs = duration_cast<microseconds>(end - start).count() / 1000.0;
    int nodes = engine.nodesSearched;
    double nps = (nodes / timeMs) * 1000.0;
    
    cout << " Done!" << endl;
    
    return {posName, depth, nodes, timeMs, nps, game.moveToString(bestMove)};
}

void printResults(const vector<BenchResult>& results) {
    cout << "\n";
    cout << "================================================================\n";
    cout << "                    BENCHMARK RESULTS                           \n";
    cout << "================================================================\n\n";
    
    cout << left << setw(25) << "Position" 
         << right << setw(6) << "Depth"
         << setw(12) << "Nodes"
         << setw(10) << "Time(ms)"
         << setw(12) << "Nodes/sec"
         << "  " << left << setw(12) << "Best Move" << endl;
    cout << string(80, '-') << endl;
    
    int totalNodes = 0;
    double totalTime = 0;
    
    for (const auto& r : results) {
        cout << left << setw(25) << r.position
             << right << setw(6) << r.depth
             << setw(12) << r.nodes
             << setw(10) << fixed << setprecision(1) << r.timeMs
             << setw(12) << fixed << setprecision(0) << r.nodesPerSec
             << "  " << left << r.bestMove << endl;
        
        totalNodes += r.nodes;
        totalTime += r.timeMs;
    }
    
    cout << string(80, '-') << endl;
    cout << "Total nodes: " << totalNodes << endl;
    cout << "Total time: " << fixed << setprecision(1) << totalTime << " ms" << endl;
    cout << "Average: " << fixed << setprecision(0) << (totalNodes / totalTime) * 1000.0 << " nodes/sec" << endl;
    cout << "================================================================\n\n";
}

int main() {
    cout << "Chess Engine Performance Benchmark\n";
    cout << "===================================\n\n";
    
    cout << "This will test the engine's search speed on various positions.\n";
    cout << "Note: Move ordering (checkmates > captures > checks) should\n";
    cout << "      result in fewer nodes searched due to better pruning.\n\n";
    
    cout << "Enter search depth (recommended 3-5): ";
    int depth;
    cin >> depth;
    
    if (depth < 1 || depth > 10) {
        cout << "Invalid depth. Using default depth 4.\n";
        depth = 4;
    }
    
    vector<BenchResult> results;
    
    // Test 1: Starting position
    {
        ChessGame game;
        results.push_back(benchmarkPosition(game, "Starting Position", depth));
    }
    
    // Test 2: Middlegame position
    {
        ChessGame game;
        // Play some moves to reach middlegame
        game.makePlayerMove("e2e4");
        game.makePlayerMove("e7e5");
        game.makePlayerMove("g1f3");
        game.makePlayerMove("b8c6");
        game.makePlayerMove("f1c4");
        game.makePlayerMove("f8c5");
        game.makePlayerMove("b1c3");
        game.makePlayerMove("g8f6");
        results.push_back(benchmarkPosition(game, "Open Middlegame", depth));
    }
    
    // Test 3: Tactical position with captures
    {
        ChessGame game;
        game.makePlayerMove("e2e4");
        game.makePlayerMove("e7e5");
        game.makePlayerMove("g1f3");
        game.makePlayerMove("b8c6");
        game.makePlayerMove("f1c4");
        game.makePlayerMove("f8c5");
        game.makePlayerMove("c2c3");
        game.makePlayerMove("g8f6");
        game.makePlayerMove("d2d4");
        game.makePlayerMove("e5d4");
        results.push_back(benchmarkPosition(game, "Tactical Position", depth));
    }
    
    // Test 4: Complex position
    {
        ChessGame game;
        game.makePlayerMove("d2d4");
        game.makePlayerMove("d7d5");
        game.makePlayerMove("c2c4");
        game.makePlayerMove("e7e6");
        game.makePlayerMove("b1c3");
        game.makePlayerMove("g8f6");
        game.makePlayerMove("c1g5");
        game.makePlayerMove("f8e7");
        game.makePlayerMove("e2e3");
        game.makePlayerMove("e8g8");
        results.push_back(benchmarkPosition(game, "Closed Position", depth));
    }
    
    // Test 5: Endgame
    {
        ChessGame game;
        // Clear most pieces for endgame test
        game.makePlayerMove("e2e4");
        game.makePlayerMove("e7e5");
        game.makePlayerMove("g1f3");
        game.makePlayerMove("g8f6");
        results.push_back(benchmarkPosition(game, "Early Game", depth));
    }
    
    printResults(results);
    
    cout << "Benchmark complete!\n";
    cout << "\nInterpretation:\n";
    cout << "- Fewer nodes = better pruning from move ordering\n";
    cout << "- Higher nodes/sec = faster search (less overhead)\n";
    cout << "- With good move ordering, expect 3-10x fewer nodes\n";
    cout << "  compared to no ordering, especially in tactical positions\n";
    
    return 0;
}
