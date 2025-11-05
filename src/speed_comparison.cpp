#include "game.hpp"
#include "engine.hpp"
#include "engine_v1.hpp"
#include "evaluation.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <vector>

using namespace std;
using namespace chrono;

struct SpeedResult {
    string version;
    string position;
    int depth;
    int nodes;
    int ttHits;
    double ttHitRate;
    double timeMs;
    double nodesPerSec;
    string bestMove;
};

// Test positions
vector<pair<string, string>> testPositions = {
    {"Starting Position", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"},
    {"Italian Game", "r1bqkb1r/pppp1ppp/2n2n2/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4"},
    {"Queen's Gambit", "rnbqkb1r/ppp2ppp/4pn2/3p4/2PP4/2N2N2/PP2PPPP/R1BQKB1R w KQkq - 0 5"},
    {"Tactical Position", "r3kb1r/pp1nqppp/2p1pn2/3p1b2/2PP4/2NBPN2/PP3PPP/R1BQK2R w KQkq - 2 8"},
};

SpeedResult testEngine(const string& version, ChessGame& game, int depth, bool isV1) {
    Evaluation eval;
    
    auto start = high_resolution_clock::now();
    Move bestMove(-1, -1, -1, -1);
    int nodes = 0;
    int ttHits = 0;
    
    if (isV1) {
        EngineV1 engine(eval);
        bestMove = engine.getBestMove(game, depth);
        nodes = engine.nodesSearched;
        ttHits = engine.ttHits;
    } else {
        Engine engine(eval);
        bestMove = engine.getBestMove(game, depth);
        nodes = engine.nodesSearched;
        ttHits = engine.ttHits;
    }
    
    auto end = high_resolution_clock::now();
    double timeMs = duration_cast<microseconds>(end - start).count() / 1000.0;
    double ttHitRate = nodes > 0 ? (100.0 * ttHits / nodes) : 0.0;
    double nps = (nodes / timeMs) * 1000.0;
    
    return {version, "", depth, nodes, ttHits, ttHitRate, timeMs, nps, game.moveToString(bestMove)};
}

void printComparison(const vector<SpeedResult>& results) {
    cout << "\n";
    cout << "================================================================================\n";
    cout << "                         ZOBRIST vs FEN SPEED COMPARISON                        \n";
    cout << "================================================================================\n\n";
    
    cout << left << setw(12) << "Version"
         << right << setw(10) << "Depth"
         << setw(12) << "Nodes"
         << setw(10) << "TT Hits"
         << setw(9) << "Hit %"
         << setw(12) << "Time(ms)"
         << setw(12) << "Nodes/sec"
         << setw(10) << "Speedup" << endl;
    cout << string(87, '-') << endl;
    
    // Print pairs of results (V1 then NEW for same position)
    for (size_t i = 0; i < results.size(); i += 2) {
        const auto& v1 = results[i];
        const auto& newVer = results[i + 1];
        
        double speedup = v1.timeMs / newVer.timeMs;
        
        cout << left << setw(12) << v1.version
             << right << setw(10) << v1.depth
             << setw(12) << v1.nodes
             << setw(10) << v1.ttHits
             << setw(8) << fixed << setprecision(1) << v1.ttHitRate << "%"
             << setw(12) << fixed << setprecision(1) << v1.timeMs
             << setw(12) << fixed << setprecision(0) << v1.nodesPerSec
             << setw(10) << "" << endl;
        
        cout << left << setw(12) << newVer.version
             << right << setw(10) << newVer.depth
             << setw(12) << newVer.nodes
             << setw(10) << newVer.ttHits
             << setw(8) << fixed << setprecision(1) << newVer.ttHitRate << "%"
             << setw(12) << fixed << setprecision(1) << newVer.timeMs
             << setw(12) << fixed << setprecision(0) << newVer.nodesPerSec
             << setw(9) << fixed << setprecision(2) << speedup << "x" << endl;
        
        cout << endl;
    }
    
    // Calculate overall speedup
    double totalV1Time = 0, totalNewTime = 0;
    for (size_t i = 0; i < results.size(); i += 2) {
        totalV1Time += results[i].timeMs;
        totalNewTime += results[i + 1].timeMs;
    }
    
    cout << string(87, '-') << endl;
    cout << "Total time - FEN-based (V1): " << fixed << setprecision(1) << totalV1Time << " ms\n";
    cout << "Total time - Zobrist (NEW): " << fixed << setprecision(1) << totalNewTime << " ms\n";
    cout << "Overall Speedup: " << fixed << setprecision(2) << (totalV1Time / totalNewTime) << "x faster\n";
    cout << "================================================================================\n\n";
}

int main() {
    cout << "Chess Engine Speed Comparison: Zobrist vs FEN-based Transposition Table\n";
    cout << "=======================================================================\n\n";
    
    cout << "This benchmark compares:\n";
    cout << "- V1: FEN string-based transposition table (slow hash, slow lookup)\n";
    cout << "- NEW: Zobrist hashing (fast hash, fast lookup)\n\n";
    
    cout << "Enter search depth (recommended 5-6): ";
    int depth;
    cin >> depth;
    
    if (depth < 1 || depth > 10) {
        cout << "Invalid depth. Using default depth 5.\n";
        depth = 5;
    }
    
    vector<SpeedResult> results;
    
    for (const auto& [posName, fen] : testPositions) {
        cout << "\nTesting: " << posName << " (depth " << depth << ")\n";
        cout << string(60, '-') << endl;
        
        // Test V1 (FEN-based)
        {
            ChessGame game;
            game.loadFEN(fen);
            cout << "  V1 (FEN-based)... " << flush;
            auto result = testEngine("V1 (FEN)", game, depth, true);
            result.position = posName;
            results.push_back(result);
            cout << "Done! (" << fixed << setprecision(1) << result.timeMs << " ms)\n";
        }
        
        // Test NEW (Zobrist)
        {
            ChessGame game;
            game.loadFEN(fen);
            cout << "  NEW (Zobrist)... " << flush;
            auto result = testEngine("NEW (Zobr)", game, depth, false);
            result.position = posName;
            results.push_back(result);
            cout << "Done! (" << fixed << setprecision(1) << result.timeMs << " ms, "
                 << fixed << setprecision(2) << (results[results.size()-2].timeMs / result.timeMs) 
                 << "x faster)\n";
        }
    }
    
    printComparison(results);
    
    cout << "Benchmark complete!\n";
    cout << "\nKey Improvements:\n";
    cout << "- Zobrist hashing: O(1) incremental updates vs O(n) FEN generation\n";
    cout << "- Integer hash keys: Fast comparison vs string comparison\n";
    cout << "- Memory efficient: 8 bytes vs ~100 bytes per position\n";
    
    return 0;
}
