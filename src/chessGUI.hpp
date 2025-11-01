#pragma once
#include <SFML/Graphics.hpp>
#include "game.hpp"
#include <memory>

class ChessGUI {
private:
    std::unique_ptr<sf::RenderWindow> window;
    ChessGame& game;
    
    // Display constants
    static constexpr int WINDOW_WIDTH = 800;
    static constexpr int WINDOW_HEIGHT = 800;
    static constexpr int BOARD_SIZE = 640;  // 8 squares * 80 pixels each
    static constexpr int SQUARE_SIZE = 80;
    static constexpr int BOARD_OFFSET_X = (WINDOW_WIDTH - BOARD_SIZE) / 2;
    static constexpr int BOARD_OFFSET_Y = (WINDOW_HEIGHT - BOARD_SIZE) / 2;
    
    // Colors
    sf::Color lightSquareColor;
    sf::Color darkSquareColor;
    sf::Color selectedSquareColor;
    sf::Color legalMoveColor;
    
    // Game state
    int selectedRow;
    int selectedCol;
    bool pieceSelected;
    std::vector<Move> legalMoves;
    
    // Fonts and text
    sf::Font font;
    bool fontLoaded;
    
public:
    ChessGUI(ChessGame& chessGame);
    ~ChessGUI() = default;
    
    // Main methods
    void run();
    bool isOpen() const;
    
private:
    // Initialization
    void initializeWindow();
    void initializeColors();
    bool loadFont();
    
    // Event handling
    void handleEvents();
    void handleMouseClick(int mouseX, int mouseY);
    
    // Rendering
    void render();
    void drawBoard();
    void drawPieces();
    void drawSelectedSquare();
    void drawLegalMoves();
    void drawUI();
    
    // Utility functions
    void screenToBoard(int screenX, int screenY, int& row, int& col);
    void boardToScreen(int row, int col, int& screenX, int& screenY);
    bool isValidSquare(int row, int col) const;
    sf::Color getPieceColor(int piece) const;
    std::string getPieceSymbol(int piece) const;
    
    // Game interaction
    void selectPiece(int row, int col);
    void tryMove(int row, int col);
    void clearSelection();
    void updateLegalMoves();
};