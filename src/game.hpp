#pragma once
#include "board.hpp"
#include "moveGeneration.hpp"
#include <string>
#include <vector>
#include <map>

using namespace std;

// Structure to store information needed to undo a move
struct UndoInfo {
    Move move;
    int capturedPiece;
    bool whiteKingMovedBefore;
    bool blackKingMovedBefore;
    bool whiteKingsideRookMovedBefore;
    bool whiteQueensideRookMovedBefore;
    bool blackKingsideRookMovedBefore;
    bool blackQueensideRookMovedBefore;
    int enPassantTargetRowBefore;
    int enPassantTargetColBefore;
    int halfmoveClockBefore;
    int fullmoveNumberBefore;
    string fenBefore;
};

class ChessGame {
private:
    bool isWhiteTurn;
    bool gameOver;
    string gameResult;
    vector<Move> gameHistory;
    
    // FEN tracking
    string currentFEN;
    int halfmoveClock;  // Moves since last capture or pawn move (for 50-move rule)
    int fullmoveNumber; // Increments after black's move
    
    // Position history for threefold repetition (position -> count)
    map<string, int> positionHistory;
    
    // Undo stack
    vector<UndoInfo> undoStack;

public:
    ChessGame();
    
    // Game control
    void startNewGame();
    bool isGameOver() const { return gameOver; }
    string getGameResult() const { return gameResult; }
    bool isWhiteToMove() const { return isWhiteTurn; }
    
    // Move handling
    bool makePlayerMove(const string& moveStr);
    bool makePlayerMove(const string& moveStr, char promotionPiece);
    void makeMoveForEngine(const Move& move);  // For engine search - saves undo info
    void undoMove();
    bool makeEngineMove(const Move& move);  // For engine to actually play a move in the game
    vector<Move> getLegalMoves() const;
    
    // Input/Output
    void displayBoard() const;
    void displayLegalMoves() const;
    Move parseMove(const string& moveStr) const;
    string moveToString(const Move& move) const;
    
    // Game status
    void updateGameStatus();
    bool isInCheck() const;
    bool isInCheckmate() const;
    bool isInStalemate() const;
    bool isDrawByRepetition() const;
    bool isDrawByFiftyMoveRule() const;
    bool isDrawByInsufficientMaterial() const;
    
    // Utility
    string coordinateToString(int row, int col) const;
    bool parseCoordinate(const string& coord, int& row, int& col) const;
    bool isPawnPromotion(int startRow, int startCol, int targetRow, int targetCol) const;
    char getPromotionChoice();
    
    // FEN handling
    string generateFEN() const;
    string getCurrentFEN() const { return currentFEN; }
    string getPositionKey() const;  // Get FEN without move counters for repetition tracking
    void updateFEN();
    void recordPosition();  // Track position for repetition detection
};