#include "game.hpp"
#include "engine.hpp"
#include "evaluation.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>

using namespace std;
using namespace chrono;

struct BenchResult {
    string testName;
    int depth;
    int nodes;
    int ttHits;
    double ttHitRate;
    double timeMs;
    double nodesPerSec;
    string details;
};

// Test 1: Iterative deepening on same position (TT should help a LOT)
BenchResult testIterativeDeepening(int maxDepth) {
    cout << "\n=== Test 1: Iterative Deepening ===" << endl;
    cout << "Searching same position repeatedly from depth 1 to " << maxDepth << endl;
    cout << "Expected: HIGH TT hit rate (reusing results from shallower searches)" << endl;
    
    ChessGame game;
    Engine engine;
    
    auto start = high_resolution_clock::now();
    int totalNodes = 0;
    int totalHits = 0;
    
    // Search same position at increasing depths (like real engine does)
    for (int d = 1; d <= maxDepth; d++) {
        engine.getBestMove(game, d);
        totalNodes += engine.nodesSearched;
        totalHits += engine.ttHits;
        cout << "  Depth " << d << ": " << engine.nodesSearched << " nodes, " 
             << engine.ttHits << " TT hits" << endl;
    }
    
    auto end = high_resolution_clock::now();
    double timeMs = duration_cast<microseconds>(end - start).count() / 1000.0;
    double hitRate = totalNodes > 0 ? (100.0 * totalHits / totalNodes) : 0.0;
    
    cout << "Total: " << totalNodes << " nodes, " << totalHits << " hits (" 
         << fixed << setprecision(1) << hitRate << "%)" << endl;
    
    return {"Iterative Deepening", maxDepth, totalNodes, totalHits, hitRate, timeMs, 
            (totalNodes / timeMs) * 1000.0, "Same position, depths 1-" + to_string(maxDepth)};
}

// Test 2: Multiple searches on same position (TT should be reused)
BenchResult testRepeatedSearch(int depth, int numSearches) {
    cout << "\n=== Test 2: Repeated Search ===" << endl;
    cout << "Searching same position " << numSearches << " times at depth " << depth << endl;
    cout << "Expected: 1st search fills TT, subsequent searches reuse it" << endl;
    
    ChessGame game;
    Engine engine;
    
    auto start = high_resolution_clock::now();
    int totalNodes = 0;
    int totalHits = 0;
    
    for (int i = 1; i <= numSearches; i++) {
        engine.getBestMove(game, depth);
        totalNodes += engine.nodesSearched;
        totalHits += engine.ttHits;
        cout << "  Search " << i << ": " << engine.nodesSearched << " nodes, " 
             << engine.ttHits << " TT hits" << endl;
    }
    
    auto end = high_resolution_clock::now();
    double timeMs = duration_cast<microseconds>(end - start).count() / 1000.0;
    double hitRate = totalNodes > 0 ? (100.0 * totalHits / totalNodes) : 0.0;
    
    cout << "Total: " << totalNodes << " nodes, " << totalHits << " hits (" 
         << fixed << setprecision(1) << hitRate << "%)" << endl;
    
    return {"Repeated Search", depth, totalNodes, totalHits, hitRate, timeMs,
            (totalNodes / timeMs) * 1000.0, to_string(numSearches) + " searches at depth " + to_string(depth)};
}

// Test 3: Transposition via move ordering (same position, different move order)
BenchResult testTranspositions(int depth) {
    cout << "\n=== Test 3: Move Transpositions ===" << endl;
    cout << "Playing different move sequences that reach same/similar positions" << endl;
    cout << "Expected: MODERATE TT hit rate (some positions appear via different move orders)" << endl;
    
    Engine engine;  // Single engine for all positions
    
    auto start = high_resolution_clock::now();
    int totalNodes = 0;
    int totalHits = 0;
    
    // Position 1: Italian Game
    {
        ChessGame game;
        game.makePlayerMove("e2e4");
        game.makePlayerMove("e7e5");
        game.makePlayerMove("g1f3");
        game.makePlayerMove("b8c6");
        game.makePlayerMove("f1c4");
        engine.getBestMove(game, depth);
        totalNodes += engine.nodesSearched;
        totalHits += engine.ttHits;
        cout << "  Italian Game: " << engine.nodesSearched << " nodes, " 
             << engine.ttHits << " hits" << endl;
    }
    
    // Position 2: Same position via different move order
    {
        ChessGame game;
        game.makePlayerMove("g1f3");  // Different order
        game.makePlayerMove("b8c6");
        game.makePlayerMove("e2e4");
        game.makePlayerMove("e7e5");
        game.makePlayerMove("f1c4");
        engine.getBestMove(game, depth);
        totalNodes += engine.nodesSearched;
        totalHits += engine.ttHits;
        cout << "  Same position (different order): " << engine.nodesSearched << " nodes, " 
             << engine.ttHits << " hits" << endl;
    }
    
    // Position 3: Similar but different
    {
        ChessGame game;
        game.makePlayerMove("e2e4");
        game.makePlayerMove("e7e5");
        game.makePlayerMove("g1f3");
        game.makePlayerMove("b8c6");
        game.makePlayerMove("f1b5");  // Spanish instead
        engine.getBestMove(game, depth);
        totalNodes += engine.nodesSearched;
        totalHits += engine.ttHits;
        cout << "  Spanish Game: " << engine.nodesSearched << " nodes, " 
             << engine.ttHits << " hits" << endl;
    }
    
    auto end = high_resolution_clock::now();
    double timeMs = duration_cast<microseconds>(end - start).count() / 1000.0;
    double hitRate = totalNodes > 0 ? (100.0 * totalHits / totalNodes) : 0.0;
    
    cout << "Total: " << totalNodes << " nodes, " << totalHits << " hits (" 
         << fixed << setprecision(1) << hitRate << "%)" << endl;
    
    return {"Transposition Test", depth, totalNodes, totalHits, hitRate, timeMs,
            (totalNodes / timeMs) * 1000.0, "3 related positions"};
}

// Test 4: Deep search (measure hit rate within single search tree)
BenchResult testDeepSearch(int depth) {
    cout << "\n=== Test 4: Single Deep Search ===" << endl;
    cout << "One deep search from starting position (depth " << depth << ")" << endl;
    cout << "Expected: LOW-MODERATE TT hit rate (transpositions within search tree only)" << endl;
    
    ChessGame game;
    Engine engine;
    
    auto start = high_resolution_clock::now();
    Move bestMove = engine.getBestMove(game, depth);
    auto end = high_resolution_clock::now();
    
    double timeMs = duration_cast<microseconds>(end - start).count() / 1000.0;
    double hitRate = engine.nodesSearched > 0 ? (100.0 * engine.ttHits / engine.nodesSearched) : 0.0;
    
    cout << "Nodes: " << engine.nodesSearched << ", TT hits: " << engine.ttHits 
         << " (" << fixed << setprecision(1) << hitRate << "%)" << endl;
    cout << "Best move: " << game.moveToString(bestMove) << endl;
    
    return {"Deep Search", depth, engine.nodesSearched, engine.ttHits, hitRate, timeMs,
            (engine.nodesSearched / timeMs) * 1000.0, "Single search at depth " + to_string(depth)};
}

void printResults(const vector<BenchResult>& results) {
    cout << "\n";
    cout << "========================================================================\n";
    cout << "                    TRANSPOSITION TABLE BENCHMARK                      \n";
    cout << "========================================================================\n\n";
    
    cout << left << setw(25) << "Test" 
         << right << setw(8) << "Depth"
         << setw(12) << "Nodes"
         << setw(12) << "TT Hits"
         << setw(10) << "Hit %"
         << setw(11) << "Time(ms)"
         << setw(12) << "Nodes/sec" << endl;
    cout << string(90, '-') << endl;
    
    int totalNodes = 0;
    int totalTTHits = 0;
    double totalTime = 0;
    
    for (const auto& r : results) {
        cout << left << setw(25) << r.testName
             << right << setw(8) << r.depth
             << setw(12) << r.nodes
             << setw(12) << r.ttHits
             << setw(9) << fixed << setprecision(1) << r.ttHitRate << "%"
             << setw(11) << fixed << setprecision(1) << r.timeMs
             << setw(12) << fixed << setprecision(0) << r.nodesPerSec << endl;
        
        totalNodes += r.nodes;
        totalTTHits += r.ttHits;
        totalTime += r.timeMs;
    }
    
    cout << string(90, '-') << endl;
    double overallHitRate = totalNodes > 0 ? (100.0 * totalTTHits / totalNodes) : 0.0;
    cout << "Total nodes: " << totalNodes << endl;
    cout << "Total TT hits: " << totalTTHits << " (" 
         << fixed << setprecision(1) << overallHitRate << "%)" << endl;
    cout << "Total time: " << fixed << setprecision(1) << totalTime << " ms" << endl;
    cout << "Average: " << fixed << setprecision(0) << (totalNodes / totalTime) * 1000.0 << " nodes/sec" << endl;
    cout << "========================================================================\n\n";
}

int main() {
    cout << "Chess Engine Transposition Table Benchmark\n";
    cout << "==========================================\n\n";
    
    cout << "This benchmark tests TT effectiveness in realistic scenarios:\n";
    cout << "1. Iterative deepening (should have high TT reuse)\n";
    cout << "2. Repeated searches (should reuse previous results)\n";
    cout << "3. Transpositions (different move orders  same position)\n";
    cout << "4. Deep single search (baseline: transpositions within one search)\n\n";
    
    cout << "Enter search depth (recommended 5-6): ";
    int depth;
    cin >> depth;
    
    if (depth < 1 || depth > 10) {
        cout << "Invalid depth. Using default depth 5.\n";
        depth = 5;
    }
    
    vector<BenchResult> results;
    
    // Test 1: Iterative deepening (should have best TT hit rate)
    results.push_back(testIterativeDeepening(depth));
    
    // Test 2: Repeated search (should reuse TT entries)
    results.push_back(testRepeatedSearch(depth, 3));
    
    // Test 3: Transpositions (different paths to similar positions)
    results.push_back(testTranspositions(depth));
    
    // Test 4: Single deep search (baseline)
    results.push_back(testDeepSearch(depth));
    
    printResults(results);
    
    cout << "Benchmark complete!\n";
    cout << "\nInterpretation:\n";
    cout << "- Iterative deepening should show highest TT hit rate\n";
    cout << "- Repeated searches should be much faster after 1st search\n";
    cout << "- Transposition test shows TT reuse across different game paths\n";
    cout << "- Deep search shows baseline hit rate (transpositions within search)\n";
    
    return 0;
}
