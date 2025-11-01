#include "chessGUI.hpp"
#include <iostream>

ChessGUI::ChessGUI(ChessGame& chessGame) 
    : game(chessGame), selectedRow(-1), selectedCol(-1), pieceSelected(false), fontLoaded(false) {
    
    initializeWindow();
    initializeColors();
    loadFont();
    updateLegalMoves();
}

void ChessGUI::initializeWindow() {
    window = std::make_unique<sf::RenderWindow>(
        sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), 
        "Chess Engine",
        sf::Style::Titlebar | sf::Style::Close
    );
    window->setFramerateLimit(60);
}

void ChessGUI::initializeColors() {
    lightSquareColor = sf::Color(240, 217, 181);  // Light brown
    darkSquareColor = sf::Color(181, 136, 99);    // Dark brown
    selectedSquareColor = sf::Color(255, 255, 0, 128);  // Semi-transparent yellow
    legalMoveColor = sf::Color(0, 255, 0, 100);   // Semi-transparent green
}

bool ChessGUI::loadFont() {
    // Try to load a system font for chess pieces (Unicode symbols)
    // On Windows, we can try to load a common font
    if (font.loadFromFile("C:/Windows/Fonts/arial.ttf") ||
        font.loadFromFile("C:/Windows/Fonts/calibri.ttf") ||
        font.loadFromFile("arial.ttf") ||
        font.loadFromFile("calibri.ttf")) {
        fontLoaded = true;
        return true;
    }
    
    std::cout << "Warning: Could not load font. Using default font.\n";
    fontLoaded = false;
    return false;
}

void ChessGUI::run() {
    while (window->isOpen()) {
        handleEvents();
        render();
    }
}

bool ChessGUI::isOpen() const {
    return window && window->isOpen();
}

void ChessGUI::handleEvents() {
    sf::Event event;
    while (window->pollEvent(event)) {
        switch (event.type) {
            case sf::Event::Closed:
                window->close();
                break;
                
            case sf::Event::MouseButtonPressed:
                if (event.mouseButton.button == sf::Mouse::Left) {
                    handleMouseClick(event.mouseButton.x, event.mouseButton.y);
                }
                break;
                
            case sf::Event::KeyPressed:
                if (event.key.code == sf::Keyboard::Escape) {
                    clearSelection();
                }
                break;
                
            default:
                break;
        }
    }
}

void ChessGUI::handleMouseClick(int mouseX, int mouseY) {
    int row, col;
    screenToBoard(mouseX, mouseY, row, col);
    
    if (!isValidSquare(row, col)) {
        return;
    }
    
    if (!pieceSelected) {
        // Try to select a piece
        selectPiece(row, col);
    } else {
        // Try to move or select different piece
        if (row == selectedRow && col == selectedCol) {
            // Clicking same square - deselect
            clearSelection();
        } else if (!isEmpty(board[row][col]) && 
                   isWhite(board[row][col]) == game.isWhiteToMove()) {
            // Clicking on another piece of same color - select it
            selectPiece(row, col);
        } else {
            // Try to move to this square
            tryMove(row, col);
        }
    }
}

void ChessGUI::render() {
    window->clear(sf::Color(50, 50, 50));  // Dark gray background
    
    drawBoard();
    drawSelectedSquare();
    drawLegalMoves();
    drawPieces();
    drawUI();
    
    window->display();
}

void ChessGUI::drawBoard() {
    sf::RectangleShape square(sf::Vector2f(SQUARE_SIZE, SQUARE_SIZE));
    
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            // Determine square color (alternating pattern)
            bool isLightSquare = (row + col) % 2 == 0;
            square.setFillColor(isLightSquare ? lightSquareColor : darkSquareColor);
            
            // Position the square
            int screenX = BOARD_OFFSET_X + col * SQUARE_SIZE;
            int screenY = BOARD_OFFSET_Y + row * SQUARE_SIZE;
            square.setPosition(screenX, screenY);
            
            window->draw(square);
        }
    }
}

void ChessGUI::drawPieces() {
    if (!fontLoaded) return;
    
    sf::Text pieceText;
    pieceText.setFont(font);
    pieceText.setCharacterSize(48);
    
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            int piece = board[row][col];
            if (isEmpty(piece)) continue;
            
            std::string symbol = getPieceSymbol(piece);
            pieceText.setString(symbol);
            pieceText.setFillColor(getPieceColor(piece));
            
            // Center the piece in the square
            sf::FloatRect textBounds = pieceText.getLocalBounds();
            int screenX = BOARD_OFFSET_X + col * SQUARE_SIZE;
            int screenY = BOARD_OFFSET_Y + row * SQUARE_SIZE;
            
            pieceText.setPosition(
                screenX + (SQUARE_SIZE - textBounds.width) / 2,
                screenY + (SQUARE_SIZE - textBounds.height) / 2 - 5
            );
            
            window->draw(pieceText);
        }
    }
}

void ChessGUI::drawSelectedSquare() {
    if (!pieceSelected) return;
    
    sf::RectangleShape highlight(sf::Vector2f(SQUARE_SIZE, SQUARE_SIZE));
    highlight.setFillColor(selectedSquareColor);
    
    int screenX = BOARD_OFFSET_X + selectedCol * SQUARE_SIZE;
    int screenY = BOARD_OFFSET_Y + selectedRow * SQUARE_SIZE;
    highlight.setPosition(screenX, screenY);
    
    window->draw(highlight);
}

void ChessGUI::drawLegalMoves() {
    if (!pieceSelected) return;
    
    sf::CircleShape moveIndicator(SQUARE_SIZE / 4);
    moveIndicator.setFillColor(legalMoveColor);
    moveIndicator.setOrigin(SQUARE_SIZE / 4, SQUARE_SIZE / 4);
    
    for (const Move& move : legalMoves) {
        if (move.startRow == selectedRow && move.startColumn == selectedCol) {
            int screenX = BOARD_OFFSET_X + move.targetColumn * SQUARE_SIZE + SQUARE_SIZE / 2;
            int screenY = BOARD_OFFSET_Y + move.targetRow * SQUARE_SIZE + SQUARE_SIZE / 2;
            
            moveIndicator.setPosition(screenX, screenY);
            window->draw(moveIndicator);
        }
    }
}

void ChessGUI::drawUI() {
    if (!fontLoaded) return;
    
    sf::Text gameInfo;
    gameInfo.setFont(font);
    gameInfo.setCharacterSize(24);
    gameInfo.setFillColor(sf::Color::White);
    
    std::string infoText = game.isWhiteToMove() ? "White to move" : "Black to move";
    
    if (game.isInCheck()) {
        infoText += " - CHECK!";
        gameInfo.setFillColor(sf::Color::Red);
    }
    
    if (game.isGameOver()) {
        infoText = game.getGameResult();
        gameInfo.setFillColor(sf::Color::Yellow);
    }
    
    gameInfo.setString(infoText);
    gameInfo.setPosition(20, 20);
    window->draw(gameInfo);
}

void ChessGUI::screenToBoard(int screenX, int screenY, int& row, int& col) {
    col = (screenX - BOARD_OFFSET_X) / SQUARE_SIZE;
    row = (screenY - BOARD_OFFSET_Y) / SQUARE_SIZE;
}

void ChessGUI::boardToScreen(int row, int col, int& screenX, int& screenY) {
    screenX = BOARD_OFFSET_X + col * SQUARE_SIZE;
    screenY = BOARD_OFFSET_Y + row * SQUARE_SIZE;
}

bool ChessGUI::isValidSquare(int row, int col) const {
    return row >= 0 && row < 8 && col >= 0 && col < 8;
}

sf::Color ChessGUI::getPieceColor(int piece) const {
    return isWhite(piece) ? sf::Color::White : sf::Color::Black;
}

std::string ChessGUI::getPieceSymbol(int piece) const {
    // Unicode chess symbols
    switch(piece) {
        case WHITE_PAWN:   return "♙";
        case WHITE_ROOK:   return "♖";
        case WHITE_KNIGHT: return "♘";
        case WHITE_BISHOP: return "♗";
        case WHITE_QUEEN:  return "♕";
        case WHITE_KING:   return "♔";
        case BLACK_PAWN:   return "♟";
        case BLACK_ROOK:   return "♜";
        case BLACK_KNIGHT: return "♞";
        case BLACK_BISHOP: return "♝";
        case BLACK_QUEEN:  return "♛";
        case BLACK_KING:   return "♚";
        default:           return " ";
    }
}

void ChessGUI::selectPiece(int row, int col) {
    int piece = board[row][col];
    
    // Check if there's a piece and it belongs to current player
    if (isEmpty(piece) || isWhite(piece) != game.isWhiteToMove()) {
        clearSelection();
        return;
    }
    
    selectedRow = row;
    selectedCol = col;
    pieceSelected = true;
    updateLegalMoves();
}

void ChessGUI::tryMove(int row, int col) {
    if (!pieceSelected) return;
    
    // Create move string and try to make the move
    std::string moveStr = game.coordinateToString(selectedRow, selectedCol) + 
                         game.coordinateToString(row, col);
    
    if (game.makePlayerMove(moveStr)) {
        clearSelection();
        updateLegalMoves();
    }
    // If move failed, keep selection (invalid move)
}

void ChessGUI::clearSelection() {
    selectedRow = -1;
    selectedCol = -1;
    pieceSelected = false;
}

void ChessGUI::updateLegalMoves() {
    legalMoves = game.getLegalMoves();
}