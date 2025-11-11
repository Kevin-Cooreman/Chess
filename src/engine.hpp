#pragma once
#include "board.hpp"
#include "game.hpp"
#include "evaluation.hpp"
#include "moveGeneration.hpp"
#include <iostream>
#include <vector>
#include <atomic>
#include <string>
#include <limits>
#include <cstdint>
#include <random>
#include <array>
#include <cstring>

// Transposition table entry
enum class TTBound : int {
    EXACT = 0,
    LOWER = 1,
    UPPER = 2
};

struct TTEntry {
    double score = 0.0;
    int depth = 0;
    TTBound bound = TTBound::EXACT; // Whether the stored score is exact, a lower bound or an upper bound
    int mateDistance = 0; // If this entry represents a mate score, store mate-in-N (plies) here. 0 = not a mate
    uint32_t packedMove = 0; // optional packed move for move ordering
};

// Utility: pack/unpack Move into a 32-bit integer for compact TT storage.
static inline uint32_t packMove(const Move &m) {
    // startRow(3) startCol(3) targetRow(3) targetCol(3) moveType(3) promotion(5)
    uint32_t p = 0;
    p |= (uint32_t)(m.startRow & 0x7);
    p |= ((uint32_t)(m.startColumn & 0x7) << 3);
    p |= ((uint32_t)(m.targetRow & 0x7) << 6);
    p |= ((uint32_t)(m.targetColumn & 0x7) << 9);
    p |= ((uint32_t)(m.moveType & 0x7) << 12);
    p |= ((uint32_t)(m.promotionPiece & 0x1F) << 15);
    return p;
}

static inline Move unpackMove(uint32_t p) {
    int sR = p & 0x7;
    int sC = (p >> 3) & 0x7;
    int tR = (p >> 6) & 0x7;
    int tC = (p >> 9) & 0x7;
    MoveType mt = static_cast<MoveType>((p >> 12) & 0x7);
    int promo = (p >> 15) & 0x1F;
    return Move(sR, sC, tR, tC, mt, promo);
}

// Simple fixed-size transposition table (2-way associative)
// Implemented inline here to avoid adding new compilation units.
class TranspositionTable {
public:
    TranspositionTable() : buckets_(0), ways_(2), curAge_(1) {}

    ~TranspositionTable() = default;

    // Initialize table with approx size in megabytes (default 64 MB)
    void init(size_t sizeMB = 256) {
        // estimate entries assuming ~32 bytes per entry
        size_t approxEntries = (sizeMB * 1024ULL * 1024ULL) / sizeof(EntryPacked);
        // increase associativity to reduce destructive collisions
        ways_ = 4;
        size_t targetBuckets = approxEntries / ways_;
        if (targetBuckets == 0) targetBuckets = 1;
        // round up to next power of two
        size_t b = 1;
        while (b < targetBuckets) b <<= 1;
        buckets_ = b;
        try {
            table_.assign(buckets_ * ways_, EntryPacked());
        } catch(...) {
            // ignore allocation failures
            table_.clear();
            buckets_ = 0;
        }
        curAge_ = 1;
    }

    // Instrumentation counters
    mutable std::atomic<uint64_t> probeCount{0};
    mutable std::atomic<uint64_t> probeHitCount{0};
    mutable std::atomic<uint64_t> storeCount{0};
    mutable std::atomic<uint64_t> replaceCount{0};
    mutable std::atomic<uint64_t> overwrittenExactCount{0};
    std::array<uint64_t, 16> storeDepthHist{{0}}; // depth histogram (0..14, 15+)

    // Probe for key; if found, fill outEntry and return true
    bool probe(uint64_t key, TTEntry &outEntry) const {
        if (buckets_ == 0 || table_.empty()) return false;
        probeCount.fetch_add(1, std::memory_order_relaxed);
        size_t idx = static_cast<size_t>(key) & (buckets_ - 1);
        size_t base = idx * ways_;
        for (size_t w = 0; w < ways_; ++w) {
            const EntryPacked &e = table_[base + w];
            if (e.key == key && e.depth > 0) {
                probeHitCount.fetch_add(1, std::memory_order_relaxed);
                outEntry.score = e.score;
                outEntry.depth = e.depth;
                outEntry.bound = static_cast<TTBound>(e.bound);
                outEntry.mateDistance = e.mateDistance;
                outEntry.packedMove = e.packedMove;
                return true;
            }
        }
        return false;
    }

    // Store an entry (replacement policy: prefer deeper entries, then older)
    void store(uint64_t key, double score, int depth, TTBound bound, int mateDistance, uint32_t packedMove = 0) {
    if (buckets_ == 0) init(256); // lazy init
        if (buckets_ == 0) return;
    storeCount.fetch_add(1, std::memory_order_relaxed);
        size_t idx = static_cast<size_t>(key) & (buckets_ - 1);
        size_t base = idx * ways_;

        // If matching key exists, update if the new depth is deeper or overwrite
        for (size_t w = 0; w < ways_; ++w) {
            EntryPacked &e = table_[base + w];
            if (e.key == key) {
                if (depth >= e.depth) {
                    e.score = score; e.depth = depth; e.bound = static_cast<uint8_t>(bound); e.mateDistance = mateDistance; e.age = curAge_; e.packedMove = packedMove;
                }
                curAge_++;
                return;
            }
        }

        // Prefer empty slot
        for (size_t w = 0; w < ways_; ++w) {
            EntryPacked &e = table_[base + w];
            if (e.key == 0) {
                e.key = key; e.score = score; e.depth = depth; e.bound = static_cast<uint8_t>(bound); e.mateDistance = mateDistance; e.age = curAge_++; e.packedMove = packedMove;
                return;
            }
        }

        // Replacement policy: avoid overwriting exact+deep entries when possible
        // Find the slot with the worst priority to replace: prefer shallower depth, older age, and non-EXACT bound
    size_t replaceIdx = 0;
        int32_t worstDepth = table_[base].depth;
        uint8_t worstAge = table_[base].age;
        uint8_t worstBound = table_[base].bound;
        for (size_t w = 1; w < ways_; ++w) {
            EntryPacked &e = table_[base + w];
            bool isWorse = false;
            if (e.depth < worstDepth) isWorse = true;
            else if (e.depth == worstDepth) {
                // prefer to replace non-EXACT entries
                if (e.bound != 0 && worstBound == 0) isWorse = true;
                else if (e.age <= worstAge) isWorse = true;
            }
            if (isWorse) {
                replaceIdx = w;
                worstDepth = e.depth;
                worstAge = e.age;
                worstBound = e.bound;
            }
        }

        // If the chosen victim is an EXACT entry and deeper than new depth, try to find any non-EXACT
        EntryPacked &victim = table_[base + replaceIdx];
        // record that we're about to replace someone
        replaceCount.fetch_add(1, std::memory_order_relaxed);
        if (victim.bound == static_cast<uint8_t>(TTBound::EXACT) && victim.depth > depth) {
            bool foundAlt = false;
            for (size_t w = 0; w < ways_; ++w) {
                EntryPacked &e = table_[base + w];
                if (e.bound != static_cast<uint8_t>(TTBound::EXACT)) {
                    replaceIdx = w; foundAlt = true; break;
                }
            }
            if (!foundAlt) {
                // keep the deeper exact entry; fall back to replacing the oldest
                size_t oldest = 0;
                uint8_t oldestAge = table_[base].age;
                for (size_t w = 1; w < ways_; ++w) {
                    if (table_[base + w].age > oldestAge) { oldest = w; oldestAge = table_[base + w].age; }
                }
                replaceIdx = oldest;
            }
        }

        // Perform replace at replaceIdx
        EntryPacked &target = table_[base + replaceIdx];
        // if victim was exact and deeper than new depth, count it
        if (target.bound == static_cast<uint8_t>(TTBound::EXACT) && target.depth > depth) {
            overwrittenExactCount.fetch_add(1, std::memory_order_relaxed);
        }
        target.key = key; target.score = score; target.depth = depth; target.bound = static_cast<uint8_t>(bound); target.mateDistance = mateDistance; target.age = curAge_++; target.packedMove = packedMove;
        // histogram of stored depths
        size_t dh = (depth >= 15) ? 15 : (size_t)depth;
        storeDepthHist[dh]++;
    }

    void printSummary() const {
        using std::cout; using std::endl;
        uint64_t probes = probeCount.load(std::memory_order_relaxed);
        uint64_t hits = probeHitCount.load(std::memory_order_relaxed);
        uint64_t stores = storeCount.load(std::memory_order_relaxed);
        uint64_t replaces = replaceCount.load(std::memory_order_relaxed);
        uint64_t overwrittenExact = overwrittenExactCount.load(std::memory_order_relaxed);
        cout << "\nTranspositionTable summary:\n";
        cout << "  probes: " << probes << ", hits: " << hits << ", hit%: ";
        if (probes) cout << (100.0 * hits / probes) << "%\n"; else cout << "0%\n";
        cout << "  stores: " << stores << ", replacements: " << replaces << ", overwrittenExact: " << overwrittenExact << "\n";
        cout << "  store depth histogram:\n";
        for (size_t i = 0; i < storeDepthHist.size(); ++i) {
            if (storeDepthHist[i] == 0) continue;
            cout << "    depth " << i << ": " << storeDepthHist[i] << "\n";
        }
    }

    // Clear table
    void clear() {
        if (table_.empty()) return;
        for (auto &e : table_) {
            e.key = 0; e.score = 0.0; e.depth = 0; e.mateDistance = 0; e.bound = 0; e.age = 0; e.packedMove = 0;
        }
        curAge_ = 1;
    }

    // Approximate number of slots
    size_t capacity() const { return buckets_ * ways_; }

private:
    struct EntryPacked {
        uint64_t key = 0;
        double score = 0.0;
        int32_t depth = 0;
        int32_t mateDistance = 0;
        uint8_t bound = 0;
        uint8_t age = 0;
        uint32_t packedMove = 0;
    };

    std::vector<EntryPacked> table_; // contiguous storage: buckets * ways
    size_t buckets_ = 0; // power-of-two
    size_t ways_ = 2;
    uint8_t curAge_ = 1;
};

class Engine {
private:
    Evaluation evaluator;
    TranspositionTable transpositionTable;
    Move pvMove = Move(-1, -1, -1, -1);  // Initialize to invalid move
    // Killer moves: two killers per ply (store packed moves)
    static constexpr int MAX_PLY = 128;
    std::array<std::array<uint32_t,2>, MAX_PLY> killers;
    // History heuristic: indexed by from*64 + to
    std::array<int, 64*64> history;

    // Helper functions
    void fastOrderMoves(vector<Move>& moves);  // Fast MVV-LVA ordering without making moves
    void orderRootMoves(ChessGame& game, vector<Move>& moves); // Order root moves, preferring checks/mates
    vector<Move> generateCaptureMoves(ChessGame& game);  // Generate only capture moves for quiescence
    void orderMovesForSearch(ChessGame& game, vector<Move>& moves, int ply);

    // Search algorithm
    // 'ply' is the number of plies from the root (used to prefer shorter mates)
    double alphabeta(ChessGame& game, int depth, double alpha, double beta, bool isMaximizing, bool allowNullMove = true, int ply = 0);
    double quiescence(ChessGame& game, double alpha, double beta, bool isMaximizing, int qDepth = 0);  // Quiescence search
    // Root mate prover: try to prove mate within maxDepth plies. If a mate is found,
    // returns true and sets outMove to the mating root move.
    bool rootMateProver(ChessGame& game, int maxDepth, Move& outMove);
    bool canForceMate(ChessGame& game, int depthLeft, bool attackerIsWhite);

public:
    Engine();
    Engine(const Evaluation& eval);
    ~Engine() = default;

    // Expose TT summary for diagnostics
    void printTTSummary() const;

    // Profiling accessors (aggregate counters collected in engine.cpp)
    static long long getTTLookupTime();   // microseconds
    static long long getEvalTime();       // microseconds
    static long long getMoveGenTime();    // microseconds
    static int getTTLookupCalls();
    static int getEvalCalls();
    static int getMoveGenCalls();
    // Make/undo profiling (microseconds)
    static void addMakeMoveTime(long long us);
    static void addUndoMoveTime(long long us);
    static long long getMakeMoveTime();
    static long long getUndoMoveTime();
    static int getMakeMoveCalls();
    static int getUndoMoveCalls();

    // Main public interface
    Move getBestMove(ChessGame& game, int depth);
    // Set RNG seed used for root move randomization (opening variety)
    static void setRngSeed(uint64_t seed);
    
    // Optional: for debugging
    int nodesSearched = 0;
    int ttHits = 0;  // Transposition table hits
};

