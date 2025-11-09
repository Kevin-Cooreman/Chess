#pragma once
#include "board.hpp"
#include "game.hpp"
#include "evaluation.hpp"
#include "moveGeneration.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <cstdint>
#include <random>

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
    TranspositionTable() : buckets_(0), curAge_(1) {}

    ~TranspositionTable() = default;

    // Initialize table with approx size in megabytes (default 64 MB)
    void init(size_t sizeMB = 64) {
        // estimate entries assuming ~32 bytes per entry
        size_t approxEntries = (sizeMB * 1024ULL * 1024ULL) / sizeof(EntryPacked);
        size_t ways = 2;
        size_t targetBuckets = approxEntries / ways;
        if (targetBuckets == 0) targetBuckets = 1;
        // round up to next power of two
        size_t b = 1;
        while (b < targetBuckets) b <<= 1;
        buckets_ = b;
        try {
            table_.assign(buckets_ * ways, EntryPacked());
        } catch(...) {
            // ignore allocation failures
            table_.clear();
            buckets_ = 0;
        }
        curAge_ = 1;
    }

    // Probe for key; if found, fill outEntry and return true
    bool probe(uint64_t key, TTEntry &outEntry) const {
        if (buckets_ == 0 || table_.empty()) return false;
        size_t idx = static_cast<size_t>(key) & (buckets_ - 1);
        size_t base = idx * 2;
        const EntryPacked &e0 = table_[base];
        if (e0.key == key && e0.depth > 0) {
            outEntry.score = e0.score;
            outEntry.depth = e0.depth;
            outEntry.bound = static_cast<TTBound>(e0.bound);
            outEntry.mateDistance = e0.mateDistance;
            outEntry.packedMove = e0.packedMove;
            return true;
        }
        const EntryPacked &e1 = table_[base + 1];
        if (e1.key == key && e1.depth > 0) {
            outEntry.score = e1.score;
            outEntry.depth = e1.depth;
            outEntry.bound = static_cast<TTBound>(e1.bound);
            outEntry.mateDistance = e1.mateDistance;
            outEntry.packedMove = e1.packedMove;
            return true;
        }
        return false;
    }

    // Store an entry (replacement policy: prefer deeper entries, then older)
    void store(uint64_t key, double score, int depth, TTBound bound, int mateDistance, uint32_t packedMove = 0) {
        if (buckets_ == 0) init(64); // lazy init
        if (buckets_ == 0) return;
        size_t idx = static_cast<size_t>(key) & (buckets_ - 1);
        size_t base = idx * 2;
        EntryPacked &e0 = table_[base];
        EntryPacked &e1 = table_[base + 1];

        // If matching key exists, update if the new depth is deeper or overwrite
        if (e0.key == key) {
            if (depth >= e0.depth) {
                e0.score = score; e0.depth = depth; e0.bound = static_cast<uint8_t>(bound); e0.mateDistance = mateDistance; e0.age = curAge_; e0.packedMove = packedMove;
            }
            curAge_++;
            return;
        }
        if (e1.key == key) {
            if (depth >= e1.depth) {
                e1.score = score; e1.depth = depth; e1.bound = static_cast<uint8_t>(bound); e1.mateDistance = mateDistance; e1.age = curAge_; e1.packedMove = packedMove;
            }
            curAge_++;
            return;
        }

        // Prefer empty slot
        if (e0.key == 0) {
            e0.key = key; e0.score = score; e0.depth = depth; e0.bound = static_cast<uint8_t>(bound); e0.mateDistance = mateDistance; e0.age = curAge_++; e0.packedMove = packedMove;
            return;
        }
        if (e1.key == 0) {
            e1.key = key; e1.score = score; e1.depth = depth; e1.bound = static_cast<uint8_t>(bound); e1.mateDistance = mateDistance; e1.age = curAge_++; e1.packedMove = packedMove;
            return;
        }

        // Replace the shallower entry; if equal depth, replace the older one
        if (e0.depth < e1.depth) {
            e0.key = key; e0.score = score; e0.depth = depth; e0.bound = static_cast<uint8_t>(bound); e0.mateDistance = mateDistance; e0.age = curAge_++; e0.packedMove = packedMove;
        } else if (e1.depth < e0.depth) {
            e1.key = key; e1.score = score; e1.depth = depth; e1.bound = static_cast<uint8_t>(bound); e1.mateDistance = mateDistance; e1.age = curAge_++; e1.packedMove = packedMove;
        } else {
            // equal depth -> replace older
            if (e0.age <= e1.age) {
                e0.key = key; e0.score = score; e0.depth = depth; e0.bound = static_cast<uint8_t>(bound); e0.mateDistance = mateDistance; e0.age = curAge_++; e0.packedMove = packedMove;
            } else {
                e1.key = key; e1.score = score; e1.depth = depth; e1.bound = static_cast<uint8_t>(bound); e1.mateDistance = mateDistance; e1.age = curAge_++; e1.packedMove = packedMove;
            }
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
    size_t capacity() const { return buckets_ * 2; }

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
    uint8_t curAge_ = 1;
};

class Engine {
private:
    Evaluation evaluator;
    TranspositionTable transpositionTable;
    Move pvMove = Move(-1, -1, -1, -1);  // Initialize to invalid move

    // Helper functions
    void fastOrderMoves(vector<Move>& moves);  // Fast MVV-LVA ordering without making moves
    void orderRootMoves(ChessGame& game, vector<Move>& moves); // Order root moves, preferring checks/mates
    vector<Move> generateCaptureMoves(ChessGame& game);  // Generate only capture moves for quiescence

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

    // Main public interface
    Move getBestMove(ChessGame& game, int depth);
    // Set RNG seed used for root move randomization (opening variety)
    static void setRngSeed(uint64_t seed);
    
    // Optional: for debugging
    int nodesSearched = 0;
    int ttHits = 0;  // Transposition table hits
};

