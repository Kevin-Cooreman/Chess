#pragma once
#include "board.hpp"
#include "moveGeneration.hpp"
#include <string>
#include <vector>

using namespace std;

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
    
    // Utility
    string coordinateToString(int row, int col) const;
    bool parseCoordinate(const string& coord, int& row, int& col) const;
    bool isPawnPromotion(int startRow, int startCol, int targetRow, int targetCol) const;
    char getPromotionChoice();
    
    // FEN handling
    string generateFEN() const;
    string getCurrentFEN() const { return currentFEN; }
    void updateFEN();
};