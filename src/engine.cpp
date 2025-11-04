#include "engine.hpp"
#include <algorithm>
#include <limits>

using namespace std;

// Get the best move for the current position
Move Engine::getBestMove(ChessGame& game, int depth) {
    nodesSearched = 0;  // Reset counter at start of search
    
    vector<Move> legalMoves = game.getLegalMoves();
    
    if (legalMoves.empty()) {
        return Move(-1, -1, -1, -1); // No legal moves
    }
    
    bool isWhiteTurn = game.isWhiteToMove();
    Move bestMove = legalMoves[0];
    
    if (isWhiteTurn) {
        // White wants to maximize evaluation
        double bestEval = -numeric_limits<double>::infinity();
        
        for (const Move& move : legalMoves) {
            game.makeMoveForEngine(move);
            double eval = alphabeta(game, depth - 1, -numeric_limits<double>::infinity(), 
                                   numeric_limits<double>::infinity(), false);
            game.undoMove();
            
            if (eval > bestEval) {
                bestEval = eval;
                bestMove = move;
            }
        }
    } else {
        // Black wants to minimize evaluation
        double bestEval = numeric_limits<double>::infinity();
        
        for (const Move& move : legalMoves) {
            game.makeMoveForEngine(move);
            double eval = alphabeta(game, depth - 1, -numeric_limits<double>::infinity(), 
                                   numeric_limits<double>::infinity(), true);
            game.undoMove();
            
            if (eval < bestEval) {
                bestEval = eval;
                bestMove = move;
            }
        }
    }
    
    // Clear undo stack after search is complete
    game.clearUndoStack();
    
    return bestMove;
}

// Order moves: checkmates first, then captures, then checks
void Engine::orderMoves(vector<Move>& moves, ChessGame& game) {
    struct MoveInfo { Move mv; int priority; int captureValue; };
    vector<MoveInfo> infos;

    for (const Move& move : moves) {
        // Detect capture by inspecting the board square at the target
        int capturedPiece = EMPTY;
        if (move.moveType == EN_PASSANT) {
            // en passant captures the pawn on the start row at target column
            capturedPiece = board[move.startRow][move.targetColumn];
        } else {
            capturedPiece = board[move.targetRow][move.targetColumn];
        }

        bool isCapture = !isEmpty(capturedPiece);
        int captureVal = 0;
        if (isCapture) {
            int pieceType = capturedPiece & 0b0111;
            switch(pieceType) {
                case 0b0001: captureVal = 1; break; // pawn
                case 0b0011: captureVal = 3; break; // knight
                case 0b0100: captureVal = 3; break; // bishop
                case 0b0010: captureVal = 5; break; // rook
                case 0b0101: captureVal = 9; break; // queen
                default: captureVal = 0; break;
            }
        }

        // Check for resulting check or mate by making the move temporarily
        bool givesCheck = false;
        bool givesMate = false;
        game.makeMoveForEngine(move);
        if (game.isInCheckmate()) givesMate = true;
        else if (game.isInCheck()) givesCheck = true;
        game.undoMove();

        int priority = 0;
        if (givesMate) priority = 3000;        // highest priority
        else if (isCapture) priority = 2000 + captureVal; // captures next, prefer higher-value targets
        else if (givesCheck) priority = 1000; // checks last
        else priority = 0;

        infos.push_back({move, priority, captureVal});
    }

    // Sort descending by priority then by capture value
    sort(infos.begin(), infos.end(), [](const MoveInfo& a, const MoveInfo& b) {
        if (a.priority != b.priority) return a.priority > b.priority;
        return a.captureValue > b.captureValue;
    });

    // Copy back
    moves.clear();
    for (const auto& mi : infos) moves.push_back(mi.mv);
}

// Alpha-beta pruning (optimized minimax) - implement later
double Engine::alphabeta(ChessGame& game, int depth, double alpha, double beta, bool isMaximizing) {
    nodesSearched++;  // Count this node
    
    if(depth == 0){
        return evaluator.evaluate(game);
    }

    vector<Move> legalmoves = game.getLegalMoves();
    
    // If no legal moves, it's checkmate or stalemate
    if(legalmoves.empty()) {
        if(game.isInCheck()) {
            // Checkmate: return extreme values
            return isMaximizing ? -100000.0 : 100000.0;
        } else {
            // Stalemate: return 0 (draw)
            return 0.0;
        }
    }
    
    // Order moves for better alpha-beta pruning
    orderMoves(legalmoves, game);
    
    if(isMaximizing){
        //white to move - maximise eval
        //initialise max Eval to -infinity (lowest possible eval)
        double maxEval = -numeric_limits<double>::infinity();

        //run through legal moves
        for(const Move& move : legalmoves){
            game.makeMoveForEngine(move);
            double eval = alphabeta(game, depth - 1, alpha, beta, false);
            game.undoMove();
            maxEval = max(maxEval, eval);
            alpha = max(alpha, eval);
            if (beta <= alpha) {
                break; // Beta cutoff
            }
        }
        
        return maxEval;
    } else {
        //black to move - minimise eval
        //initialise min Eval to +infinity (highest possible eval)
        double minEval = numeric_limits<double>::infinity();

        //run through legal moves
        for(const Move& move : legalmoves){
            game.makeMoveForEngine(move);
            double eval = alphabeta(game, depth - 1, alpha, beta, true);
            game.undoMove();
            minEval = min(minEval, eval);
            beta = min(beta, eval);
            if (beta <= alpha) {
                break; // Alpha cutoff
            }
        }
        
        return minEval;
    }
}