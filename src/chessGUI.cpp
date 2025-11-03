#include "chessGUI.hpp"
#include "evaluation.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

ChessGUI::ChessGUI(ChessGame& chessGame) 
    : game(chessGame), selectedRow(-1), selectedCol(-1), pieceSelected(false),
      showPromotionDialog(false), promotionRow(-1), promotionCol(-1),
      promotionTargetRow(-1), promotionTargetCol(-1), isWhitePromotion(true),
      fontLoaded(false), texturesLoaded(false) {
    
    initializeWindow();
    initializeColors();
    loadFont();
    texturesLoaded = loadPieceTextures();
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
    // Try to load fonts that support Unicode chess symbols
    // These fonts are more likely to have proper chess piece glyphs
    if (font.loadFromFile("C:/Windows/Fonts/seguisym.ttf") ||      // Segoe UI Symbol (best for Unicode)
        font.loadFromFile("C:/Windows/Fonts/arial.ttf") ||
        font.loadFromFile("C:/Windows/Fonts/calibri.ttf") ||
        font.loadFromFile("C:/Windows/Fonts/consola.ttf") ||       // Consolas (monospace, good for symbols)
        font.loadFromFile("C:/Windows/Fonts/times.ttf")) {
        fontLoaded = true;
        std::cout << "Font loaded successfully for chess pieces.\n";
        return true;
    }
    
    std::cout << "Warning: Could not load system font. Using SFML default font.\n";
    std::cout << "Chess pieces will be displayed as text letters.\n";
    fontLoaded = false;
    return false;
}

bool ChessGUI::loadPieceTextures() {
    // Define piece filenames
    std::map<int, std::string> pieceFiles = {
        {WHITE_PAWN,   "assets/pieces/white_pawn.png"},
        {WHITE_ROOK,   "assets/pieces/white_rook.png"},
        {WHITE_KNIGHT, "assets/pieces/white_knight.png"},
        {WHITE_BISHOP, "assets/pieces/white_bishop.png"},
        {WHITE_QUEEN,  "assets/pieces/white_queen.png"},
        {WHITE_KING,   "assets/pieces/white_king.png"},
        {BLACK_PAWN,   "assets/pieces/black_pawn.png"},
        {BLACK_ROOK,   "assets/pieces/black_rook.png"},
        {BLACK_KNIGHT, "assets/pieces/black_knight.png"},
        {BLACK_BISHOP, "assets/pieces/black_bishop.png"},
        {BLACK_QUEEN,  "assets/pieces/black_queen.png"},
        {BLACK_KING,   "assets/pieces/black_king.png"}
    };
    
    int loadedCount = 0;
    for (const auto& pair : pieceFiles) {
        if (pieceTextures[pair.first].loadFromFile(pair.second)) {
            loadedCount++;
        } else {
            std::cout << "Warning: Could not load " << pair.second << std::endl;
        }
    }
    
    if (loadedCount > 0) {
        std::cout << "Loaded " << loadedCount << "/12 piece textures successfully.\n";
        return loadedCount == 12; // Return true only if all textures loaded
    } else {
        std::cout << "No piece textures loaded. Using geometric shapes.\n";
        return false;
    }
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
    // Check if promotion dialog is showing and handle clicks on it
    if (showPromotionDialog) {
        const int dialogWidth = 320;
        const int dialogHeight = 120;
        const int dialogX = (WINDOW_WIDTH - dialogWidth) / 2;
        const int dialogY = (WINDOW_HEIGHT - dialogHeight) / 2;
        const int pieceSize = 60;
        const int startX = dialogX + 20;
        const int startY = dialogY + 45;
        
        // Check if click is within any of the piece options
        for (int i = 0; i < 4; i++) {
            int x = startX + i * (pieceSize + 10);
            int y = startY;
            
            if (mouseX >= x && mouseX <= x + pieceSize && 
                mouseY >= y && mouseY <= y + pieceSize) {
                handlePromotionChoice(i);
                return;
            }
        }
        
        // Click outside dialog - ignore
        return;
    }

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
    drawPromotionDialog();
    
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
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            int piece = board[row][col];
            if (isEmpty(piece)) continue;
            
            int screenX = BOARD_OFFSET_X + col * SQUARE_SIZE;
            int screenY = BOARD_OFFSET_Y + row * SQUARE_SIZE;
            
            // Try to use PNG textures first
            if (texturesLoaded && pieceTextures.find(piece) != pieceTextures.end()) {
                sf::Sprite pieceSprite;
                pieceSprite.setTexture(pieceTextures[piece]);
                
                // Scale the sprite to fit nicely in the square
                sf::Vector2u textureSize = pieceTextures[piece].getSize();
                float scaleX = (SQUARE_SIZE * 0.8f) / textureSize.x;
                float scaleY = (SQUARE_SIZE * 0.8f) / textureSize.y;
                float scale = std::min(scaleX, scaleY); // Use smaller scale to fit
                
                pieceSprite.setScale(scale, scale);
                
                // Center the sprite in the square
                sf::FloatRect spriteBounds = pieceSprite.getLocalBounds();
                pieceSprite.setPosition(
                    screenX + (SQUARE_SIZE - spriteBounds.width * scale) / 2,
                    screenY + (SQUARE_SIZE - spriteBounds.height * scale) / 2
                );
                
                window->draw(pieceSprite);
            } else {
                // Fallback to geometric shapes if textures not available
                drawGeometricPiece(piece, screenX, screenY);
            }
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
    sf::Text gameInfo;
    
    // Use loaded font if available, otherwise use SFML's default font
    if (fontLoaded) {
        gameInfo.setFont(font);
        gameInfo.setCharacterSize(24);
    } else {
        gameInfo.setCharacterSize(20);  // Slightly smaller for default font
    }
    
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
    
    // Display evaluation
    if (!game.isGameOver()) {
        double eval = evaluation(game);
        
        sf::Text evalText;
        if (fontLoaded) {
            evalText.setFont(font);
            evalText.setCharacterSize(20);
        } else {
            evalText.setCharacterSize(18);
        }
        
        // Format evaluation score
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2);
        
        if (eval > 0) {
            ss << "White +";
            evalText.setFillColor(sf::Color::White);
        } else if (eval < 0) {
            ss << "Black +";
            evalText.setFillColor(sf::Color(180, 180, 180));
            eval = -eval; // Make positive for display
        } else {
            ss << "Equal ";
            evalText.setFillColor(sf::Color(150, 150, 150));
        }
        
        ss << eval;
        evalText.setString("Eval: " + ss.str());
        evalText.setPosition(20, 50);
        window->draw(evalText);
    }
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
    // Use colors that contrast well with the board
    return isWhite(piece) ? sf::Color(255, 255, 255) : sf::Color(30, 30, 30);
}

std::string ChessGUI::getPieceSymbol(int piece) const {
    // For now, use clear text letters that we know work
    // We can try different Unicode symbols or use geometric shapes instead
    switch(piece) {
        case WHITE_PAWN:   return "P";
        case WHITE_ROOK:   return "R";
        case WHITE_KNIGHT: return "N";
        case WHITE_BISHOP: return "B";
        case WHITE_QUEEN:  return "Q";
        case WHITE_KING:   return "K";
        case BLACK_PAWN:   return "p";
        case BLACK_ROOK:   return "r";
        case BLACK_KNIGHT: return "n";
        case BLACK_BISHOP: return "b";
        case BLACK_QUEEN:  return "q";
        case BLACK_KING:   return "k";
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
    
    // Check if this is a pawn promotion move
    if (isPromotionMove(selectedRow, selectedCol, row, col)) {
        displayPromotionDialog(selectedRow, selectedCol, row, col, game.isWhiteToMove());
        return;
    }
    
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

void ChessGUI::drawGeometricPiece(int piece, int screenX, int screenY) {
    const float pieceSize = SQUARE_SIZE * 0.7f;
    const float centerX = screenX + SQUARE_SIZE / 2.0f;
    const float centerY = screenY + SQUARE_SIZE / 2.0f;
    
    // Better colors with more contrast
    sf::Color pieceColor = isWhite(piece) ? sf::Color(240, 240, 240) : sf::Color(40, 40, 40);
    sf::Color outlineColor = isWhite(piece) ? sf::Color(60, 60, 60) : sf::Color(200, 200, 200);
    
    int pieceType = piece & 0b0111;
    
    switch(pieceType) {
        case 0b0001: { // Pawn - circle with base
            // Main body
            sf::CircleShape pawn(pieceSize * 0.25f);
            pawn.setFillColor(pieceColor);
            pawn.setOutlineColor(outlineColor);
            pawn.setOutlineThickness(3);
            pawn.setOrigin(pieceSize * 0.25f, pieceSize * 0.25f);
            pawn.setPosition(centerX, centerY - 5);
            window->draw(pawn);
            
            // Base
            sf::RectangleShape base(sf::Vector2f(pieceSize * 0.4f, pieceSize * 0.15f));
            base.setFillColor(pieceColor);
            base.setOutlineColor(outlineColor);
            base.setOutlineThickness(2);
            base.setOrigin(pieceSize * 0.2f, pieceSize * 0.075f);
            base.setPosition(centerX, centerY + pieceSize * 0.2f);
            window->draw(base);
            break;
        }
        case 0b0010: { // Rook - castle-like rectangle with merlons
            // Main body
            sf::RectangleShape rook(sf::Vector2f(pieceSize * 0.5f, pieceSize * 0.7f));
            rook.setFillColor(pieceColor);
            rook.setOutlineColor(outlineColor);
            rook.setOutlineThickness(3);
            rook.setOrigin(pieceSize * 0.25f, pieceSize * 0.35f);
            rook.setPosition(centerX, centerY);
            window->draw(rook);
            
            // Merlons (castle battlements)
            for (int i = 0; i < 3; i++) {
                sf::RectangleShape merlon(sf::Vector2f(pieceSize * 0.12f, pieceSize * 0.15f));
                merlon.setFillColor(pieceColor);
                merlon.setOutlineColor(outlineColor);
                merlon.setOutlineThickness(2);
                merlon.setOrigin(pieceSize * 0.06f, pieceSize * 0.075f);
                merlon.setPosition(centerX - pieceSize * 0.18f + i * pieceSize * 0.18f, centerY - pieceSize * 0.42f);
                window->draw(merlon);
            }
            break;
        }
        case 0b0011: { // Knight - horse head shape
            sf::CircleShape knight(pieceSize * 0.3f, 6);
            knight.setFillColor(pieceColor);
            knight.setOutlineColor(outlineColor);
            knight.setOutlineThickness(3);
            knight.setOrigin(pieceSize * 0.3f, pieceSize * 0.3f);
            knight.setPosition(centerX, centerY);
            knight.setRotation(30);
            window->draw(knight);
            
            // Ear/mane
            sf::CircleShape ear(pieceSize * 0.1f);
            ear.setFillColor(outlineColor);
            ear.setOrigin(pieceSize * 0.1f, pieceSize * 0.1f);
            ear.setPosition(centerX - 8, centerY - 15);
            window->draw(ear);
            break;
        }
        case 0b0100: { // Bishop - pointed hat shape
            // Main body
            sf::CircleShape bishop(pieceSize * 0.3f);
            bishop.setFillColor(pieceColor);
            bishop.setOutlineColor(outlineColor);
            bishop.setOutlineThickness(3);
            bishop.setOrigin(pieceSize * 0.3f, pieceSize * 0.3f);
            bishop.setPosition(centerX, centerY + 5);
            window->draw(bishop);
            
            // Mitre (pointed top)
            sf::CircleShape mitre(pieceSize * 0.15f, 3);
            mitre.setFillColor(pieceColor);
            mitre.setOutlineColor(outlineColor);
            mitre.setOutlineThickness(2);
            mitre.setOrigin(pieceSize * 0.15f, pieceSize * 0.15f);
            mitre.setPosition(centerX, centerY - pieceSize * 0.25f);
            window->draw(mitre);
            break;
        }
        case 0b0101: { // Queen - circle with multiple crown points
            // Main body
            sf::CircleShape queen(pieceSize * 0.35f);
            queen.setFillColor(pieceColor);
            queen.setOutlineColor(outlineColor);
            queen.setOutlineThickness(3);
            queen.setOrigin(pieceSize * 0.35f, pieceSize * 0.35f);
            queen.setPosition(centerX, centerY);
            window->draw(queen);
            
            // Crown points (8 points for queen)
            for (int i = 0; i < 8; i++) {
                float angle = i * 45.0f * M_PI / 180.0f;
                sf::CircleShape point(4);
                point.setFillColor(outlineColor);
                point.setOrigin(4, 4);
                point.setPosition(centerX + cos(angle) * pieceSize * 0.45f, 
                                centerY + sin(angle) * pieceSize * 0.45f);
                window->draw(point);
            }
            break;
        }
        case 0b0110: { // King - circle with cross crown
            // Main body
            sf::CircleShape king(pieceSize * 0.35f);
            king.setFillColor(pieceColor);
            king.setOutlineColor(outlineColor);
            king.setOutlineThickness(3);
            king.setOrigin(pieceSize * 0.35f, pieceSize * 0.35f);
            king.setPosition(centerX, centerY);
            window->draw(king);
            
            // Cross on top
            sf::RectangleShape crossV(sf::Vector2f(6, pieceSize * 0.4f));
            crossV.setFillColor(outlineColor);
            crossV.setOrigin(3, pieceSize * 0.2f);
            crossV.setPosition(centerX, centerY - pieceSize * 0.3f);
            window->draw(crossV);
            
            sf::RectangleShape crossH(sf::Vector2f(pieceSize * 0.25f, 6));
            crossH.setFillColor(outlineColor);
            crossH.setOrigin(pieceSize * 0.125f, 3);
            crossH.setPosition(centerX, centerY - pieceSize * 0.3f);
            window->draw(crossH);
            break;
        }
    }
}

bool ChessGUI::isPromotionMove(int fromRow, int fromCol, int toRow, int toCol) const {
    int piece = board[fromRow][fromCol];
    
    // Check if it's a pawn
    if ((piece & 0b0111) != 0b0001) return false;
    
    // Check if pawn reaches promotion rank
    bool isWhitePawn = isWhite(piece);
    
    // Suppress unused parameter warning
    (void)toCol;
    
    return (isWhitePawn && toRow == 0) || (!isWhitePawn && toRow == 7);
}

void ChessGUI::displayPromotionDialog(int fromRow, int fromCol, int toRow, int toCol, bool isWhite) {
    showPromotionDialog = true;
    promotionRow = fromRow;
    promotionCol = fromCol;
    promotionTargetRow = toRow;
    promotionTargetCol = toCol;
    isWhitePromotion = isWhite;
}

void ChessGUI::handlePromotionChoice(int choice) {
    if (!showPromotionDialog) return;
    
    char promotionPiece;
    switch(choice) {
        case 0: promotionPiece = 'q'; break; // Queen
        case 1: promotionPiece = 'r'; break; // Rook
        case 2: promotionPiece = 'b'; break; // Bishop
        case 3: promotionPiece = 'n'; break; // Knight
        default: return; // Invalid choice
    }
    
    // Create move string and make the move with promotion
    std::string moveStr = game.coordinateToString(promotionRow, promotionCol) + 
                         game.coordinateToString(promotionTargetRow, promotionTargetCol);
    
    if (game.makePlayerMove(moveStr, promotionPiece)) {
        clearSelection();
        updateLegalMoves();
    }
    
    // Hide the promotion dialog
    showPromotionDialog = false;
}

void ChessGUI::drawPromotionDialog() {
    if (!showPromotionDialog) return;
    
    // Draw a semi-transparent overlay
    sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 128));
    window->draw(overlay);
    
    // Dialog dimensions
    const int dialogWidth = 320;
    const int dialogHeight = 120;
    const int dialogX = (WINDOW_WIDTH - dialogWidth) / 2;
    const int dialogY = (WINDOW_HEIGHT - dialogHeight) / 2;
    
    // Draw dialog background
    sf::RectangleShape dialog(sf::Vector2f(dialogWidth, dialogHeight));
    dialog.setPosition(dialogX, dialogY);
    dialog.setFillColor(sf::Color(240, 240, 240));
    dialog.setOutlineThickness(2);
    dialog.setOutlineColor(sf::Color(80, 80, 80));
    window->draw(dialog);
    
    // Draw title
    sf::Text title("Choose promotion piece:", font, 20);
    title.setFillColor(sf::Color::Black);
    title.setPosition(dialogX + 10, dialogY + 10);
    if (fontLoaded) {
        window->draw(title);
    }
    
    // Draw piece options (4 pieces in a row)
    const int pieceSize = 60;
    const int startX = dialogX + 20;
    const int startY = dialogY + 45;
    
    // Queen, Rook, Bishop, Knight
    std::vector<int> pieces;
    if (isWhitePromotion) {
        pieces = {WHITE_QUEEN, WHITE_ROOK, WHITE_BISHOP, WHITE_KNIGHT};
    } else {
        pieces = {BLACK_QUEEN, BLACK_ROOK, BLACK_BISHOP, BLACK_KNIGHT};
    }
    
    for (int i = 0; i < 4; i++) {
        int x = startX + i * (pieceSize + 10);
        int y = startY;
        
        // Draw background for piece
        sf::RectangleShape pieceBg(sf::Vector2f(pieceSize, pieceSize));
        pieceBg.setPosition(x, y);
        pieceBg.setFillColor(sf::Color(200, 200, 200));
        pieceBg.setOutlineThickness(1);
        pieceBg.setOutlineColor(sf::Color(100, 100, 100));
        window->draw(pieceBg);
        
        // Draw the piece (try texture first, then geometric fallback)
        int piece = pieces[i];
        if (texturesLoaded && pieceTextures.find(piece) != pieceTextures.end()) {
            sf::Sprite pieceSprite;
            pieceSprite.setTexture(pieceTextures[piece]);
            
            // Scale sprite to fit
            sf::Vector2u textureSize = pieceTextures[piece].getSize();
            float scale = (pieceSize * 0.8f) / std::max(textureSize.x, textureSize.y);
            pieceSprite.setScale(scale, scale);
            
            // Center the sprite
            sf::FloatRect spriteBounds = pieceSprite.getLocalBounds();
            pieceSprite.setPosition(
                x + (pieceSize - spriteBounds.width * scale) / 2,
                y + (pieceSize - spriteBounds.height * scale) / 2
            );
            
            window->draw(pieceSprite);
        } else {
            // Fallback to geometric shape
            drawGeometricPiece(piece, x, y);
        }
    }
}