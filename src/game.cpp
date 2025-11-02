#include "game.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>

using namespace std;

ChessGame::ChessGame() {
    startNewGame();
}

void ChessGame::startNewGame() {
    initBoard();
    setupStartingPosition();
    isWhiteTurn = true;
    gameOver = false;
    gameResult = "";
    gameHistory.clear();
    
    // Reset game state variables
    whiteKingMoved = false;
    blackKingMoved = false;
    whiteKingsideRookMoved = false;
    whiteQueensideRookMoved = false;
    blackKingsideRookMoved = false;
    blackQueensideRookMoved = false;
    enPassantTargetRow = -1;
    enPassantTargetCol = -1;
    
    // Initialize FEN tracking
    halfmoveClock = 0;
    fullmoveNumber = 1;
    updateFEN();
}

void ChessGame::displayBoard() const {
    cout << "\n";
    printBoard();
    cout << "\n" << (isWhiteTurn ? "White" : "Black") << " to move\n";
    
    if (isInCheck()) {
        cout << "** CHECK! **\n";
    }
    
    cout << "FEN: " << currentFEN << "\n";
    
    cout << "\n";
}

vector<Move> ChessGame::getLegalMoves() const {
    return generateLegalMoves(isWhiteTurn);
}

bool ChessGame::makePlayerMove(const string& moveStr) {
    if (gameOver) {
        cout << "Game is over! " << gameResult << endl;
        return false;
    }
    
    Move playerMove = parseMove(moveStr);
    
    // Check if it's a valid move structure
    if (playerMove.startRow == -1) {
        cout << "Invalid move format! Use format like 'e2e4' or 'e2-e4'\n";
        return false;
    }
    
    // Check if this is a pawn promotion and no promotion piece was specified
    if (isPawnPromotion(playerMove.startRow, playerMove.startColumn, 
                       playerMove.targetRow, playerMove.targetColumn) &&
        playerMove.moveType != PAWN_PROMOTION) {
        
        // Ask user for promotion choice
        char promoChoice = getPromotionChoice();
        
        // Update the move with the promotion piece
        playerMove.moveType = PAWN_PROMOTION;
        switch(promoChoice) {
            case 'q':
                playerMove.promotionPiece = isWhiteTurn ? WHITE_QUEEN : BLACK_QUEEN;
                break;
            case 'r':
                playerMove.promotionPiece = isWhiteTurn ? WHITE_ROOK : BLACK_ROOK;
                break;
            case 'b':
                playerMove.promotionPiece = isWhiteTurn ? WHITE_BISHOP : BLACK_BISHOP;
                break;
            case 'n':
                playerMove.promotionPiece = isWhiteTurn ? WHITE_KNIGHT : BLACK_KNIGHT;
                break;
        }
    }
    
    // Get all legal moves
    vector<Move> legalMoves = getLegalMoves();
    
    // Find matching legal move
    Move* matchingMove = nullptr;
    for (Move& move : legalMoves) {
        if (move.startRow == playerMove.startRow && 
            move.startColumn == playerMove.startColumn &&
            move.targetRow == playerMove.targetRow && 
            move.targetColumn == playerMove.targetColumn) {
            
            // For pawn promotion, we need to handle the promotion piece
            if (move.moveType == PAWN_PROMOTION) {
                // Default to queen promotion if not specified
                if (playerMove.moveType != PAWN_PROMOTION) {
                    int queenPiece = isWhiteTurn ? WHITE_QUEEN : BLACK_QUEEN;
                    if (move.promotionPiece == queenPiece) {
                        matchingMove = &move;
                        break;
                    }
                } else if (move.promotionPiece == playerMove.promotionPiece) {
                    matchingMove = &move;
                    break;
                }
            } else {
                matchingMove = &move;
                break;
            }
        }
    }
    
    if (!matchingMove) {
        cout << "Illegal move! Try again.\n";
        return false;
    }
    
    // Check if this move resets the halfmove clock (capture or pawn move)
    int movingPiece = board[matchingMove->startRow][matchingMove->startColumn];
    int capturedPiece = board[matchingMove->targetRow][matchingMove->targetColumn];
    bool isPawnMove = (movingPiece & 0b0111) == 0b0001;
    bool isCapture = !isEmpty(capturedPiece) || matchingMove->moveType == EN_PASSANT;
    
    if (isPawnMove || isCapture) {
        halfmoveClock = 0;
    } else {
        halfmoveClock++;
    }
    
    // Execute the move
    makeMove(*matchingMove);
    gameHistory.push_back(*matchingMove);
    
    // Switch turns
    isWhiteTurn = !isWhiteTurn;
    
    // Increment fullmove number after black's move
    if (isWhiteTurn) {
        fullmoveNumber++;
    }
    
    // Update FEN string
    updateFEN();
    
    // Update game status
    updateGameStatus();
    
    cout << "Move: " << moveToString(*matchingMove) << endl;
    
    return true;
}

bool ChessGame::makePlayerMove(const string& moveStr, char promotionPiece) {
    if (gameOver) {
        cout << "Game is over! " << gameResult << endl;
        return false;
    }
    
    Move playerMove = parseMove(moveStr);
    
    // Check if it's a valid move structure
    if (playerMove.startRow == -1) {
        cout << "Invalid move format! Use format like 'e2e4' or 'e2-e4'\n";
        return false;
    }
    
    // Check if this is a pawn promotion and set the promotion piece
    if (isPawnPromotion(playerMove.startRow, playerMove.startColumn, 
                       playerMove.targetRow, playerMove.targetColumn)) {
        
        // Update the move with the promotion piece
        playerMove.moveType = PAWN_PROMOTION;
        switch(promotionPiece) {
            case 'q':
                playerMove.promotionPiece = isWhiteTurn ? WHITE_QUEEN : BLACK_QUEEN;
                break;
            case 'r':
                playerMove.promotionPiece = isWhiteTurn ? WHITE_ROOK : BLACK_ROOK;
                break;
            case 'b':
                playerMove.promotionPiece = isWhiteTurn ? WHITE_BISHOP : BLACK_BISHOP;
                break;
            case 'n':
                playerMove.promotionPiece = isWhiteTurn ? WHITE_KNIGHT : BLACK_KNIGHT;
                break;
            default:
                cout << "Invalid promotion piece! Using queen by default.\n";
                playerMove.promotionPiece = isWhiteTurn ? WHITE_QUEEN : BLACK_QUEEN;
                break;
        }
    }
    
    // Get all legal moves
    vector<Move> legalMoves = getLegalMoves();
    
    // Find matching legal move
    Move* matchingMove = nullptr;
    for (Move& move : legalMoves) {
        if (move.startRow == playerMove.startRow && 
            move.startColumn == playerMove.startColumn &&
            move.targetRow == playerMove.targetRow && 
            move.targetColumn == playerMove.targetColumn) {
            
            // For pawn promotion, match the specific promotion piece
            if (move.moveType == PAWN_PROMOTION) {
                if (move.promotionPiece == playerMove.promotionPiece) {
                    matchingMove = &move;
                    break;
                }
            } else {
                matchingMove = &move;
                break;
            }
        }
    }
    
    if (!matchingMove) {
        cout << "Illegal move! Try again.\n";
        return false;
    }
    
    // Check if this move resets the halfmove clock (capture or pawn move)
    int movingPiece = board[matchingMove->startRow][matchingMove->startColumn];
    int capturedPiece = board[matchingMove->targetRow][matchingMove->targetColumn];
    bool isPawnMove = (movingPiece & 0b0111) == 0b0001;
    bool isCapture = !isEmpty(capturedPiece) || matchingMove->moveType == EN_PASSANT;
    
    if (isPawnMove || isCapture) {
        halfmoveClock = 0;
    } else {
        halfmoveClock++;
    }
    
    // Execute the move
    makeMove(*matchingMove);
    gameHistory.push_back(*matchingMove);
    
    // Switch turns
    isWhiteTurn = !isWhiteTurn;
    
    // Increment fullmove number after black's move
    if (isWhiteTurn) {
        fullmoveNumber++;
    }
    
    // Update FEN string
    updateFEN();
    
    // Update game status
    updateGameStatus();
    
    cout << "Move: " << moveToString(*matchingMove) << endl;
    
    return true;
}

Move ChessGame::parseMove(const string& moveStr) const {
    string cleaned = moveStr;
    
    // Remove common separators and convert to lowercase
    cleaned.erase(remove(cleaned.begin(), cleaned.end(), '-'), cleaned.end());
    cleaned.erase(remove(cleaned.begin(), cleaned.end(), ' '), cleaned.end());
    transform(cleaned.begin(), cleaned.end(), cleaned.begin(), ::tolower);
    
    if (cleaned.length() < 4) {
        return Move(-1, -1, -1, -1); // Invalid move
    }
    
    // Parse source square
    int startRow, startCol, targetRow, targetCol;
    if (!parseCoordinate(cleaned.substr(0, 2), startRow, startCol) ||
        !parseCoordinate(cleaned.substr(2, 2), targetRow, targetCol)) {
        return Move(-1, -1, -1, -1); // Invalid move
    }
    
    // Check for promotion (5th character)
    MoveType moveType = NORMAL;
    int promotionPiece = 0;
    
    if (cleaned.length() >= 5) {
        char promoChar = cleaned[4];
        if (promoChar == 'q') {
            moveType = PAWN_PROMOTION;
            promotionPiece = isWhiteTurn ? WHITE_QUEEN : BLACK_QUEEN;
        } else if (promoChar == 'r') {
            moveType = PAWN_PROMOTION;
            promotionPiece = isWhiteTurn ? WHITE_ROOK : BLACK_ROOK;
        } else if (promoChar == 'b') {
            moveType = PAWN_PROMOTION;
            promotionPiece = isWhiteTurn ? WHITE_BISHOP : BLACK_BISHOP;
        } else if (promoChar == 'n') {
            moveType = PAWN_PROMOTION;
            promotionPiece = isWhiteTurn ? WHITE_KNIGHT : BLACK_KNIGHT;
        }
    }
    
    return Move(startRow, startCol, targetRow, targetCol, moveType, promotionPiece);
}

bool ChessGame::parseCoordinate(const string& coord, int& row, int& col) const {
    if (coord.length() != 2) return false;
    
    char file = coord[0];
    char rank = coord[1];
    
    if (file < 'a' || file > 'h' || rank < '1' || rank > '8') {
        return false;
    }
    
    col = file - 'a';  // a=0, b=1, ..., h=7
    row = 8 - (rank - '0');  // 1=7, 2=6, ..., 8=0 (because row 0 is rank 8)
    
    return true;
}

string ChessGame::coordinateToString(int row, int col) const {
    if (row < 0 || row > 7 || col < 0 || col > 7) return "??";
    
    char file = 'a' + col;
    char rank = '8' - row;
    
    return string(1, file) + string(1, rank);
}

string ChessGame::moveToString(const Move& move) const {
    string result = coordinateToString(move.startRow, move.startColumn) + 
                   coordinateToString(move.targetRow, move.targetColumn);
    
    if (move.moveType == PAWN_PROMOTION) {
        int pieceType = move.promotionPiece & 0b0111;
        switch(pieceType) {
            case 0b0101: result += "q"; break; // Queen
            case 0b0010: result += "r"; break; // Rook
            case 0b0100: result += "b"; break; // Bishop
            case 0b0011: result += "n"; break; // Knight
        }
    } else if (move.moveType == CASTLING_KINGSIDE) {
        result += " (O-O)";
    } else if (move.moveType == CASTLING_QUEENSIDE) {
        result += " (O-O-O)";
    } else if (move.moveType == EN_PASSANT) {
        result += " (en passant)";
    }
    
    return result;
}

void ChessGame::displayLegalMoves() const {
    vector<Move> moves = getLegalMoves();
    
    cout << "Legal moves (" << moves.size() << "):\n";
    
    int count = 0;
    for (const Move& move : moves) {
        cout << moveToString(move) << "  ";
        count++;
        if (count % 8 == 0) cout << "\n";
    }
    if (count % 8 != 0) cout << "\n";
}

bool ChessGame::isInCheck() const {
    return isKingInCheck(isWhiteTurn);
}

bool ChessGame::isInCheckmate() const {
    return isInCheck() && getLegalMoves().empty();
}

bool ChessGame::isInStalemate() const {
    return !isInCheck() && getLegalMoves().empty();
}

void ChessGame::updateGameStatus() {
    if (isInCheckmate()) {
        gameOver = true;
        gameResult = (isWhiteTurn ? "Black" : "White") + string(" wins by checkmate!");
    } else if (isInStalemate()) {
        gameOver = true;
        gameResult = "Draw by stalemate!";
    }
}

bool ChessGame::isPawnPromotion(int startRow, int startCol, int targetRow, int targetCol) const {
    int piece = board[startRow][startCol];
    
    // Check if it's a pawn
    if ((piece & 0b0111) != 0b0001) return false;
    
    // Check if pawn reaches promotion rank
    bool isWhitePawn = isWhite(piece);
    return (isWhitePawn && targetRow == 0) || (!isWhitePawn && targetRow == 7);
    
    // Note: targetCol parameter kept for potential future use (captures, etc.)
    (void)targetCol; // Suppress unused parameter warning
}

char ChessGame::getPromotionChoice() {
    char choice;
    cout << "\nPawn promotion! Choose piece:\n";
    cout << "  q - Queen\n";
    cout << "  r - Rook\n";
    cout << "  b - Bishop\n";
    cout << "  n - Knight\n";
    cout << "Enter choice (q/r/b/n): ";
    
    while (true) {
        cin >> choice;
        choice = tolower(choice);
        
        if (choice == 'q' || choice == 'r' || choice == 'b' || choice == 'n') {
            // Clear the input buffer for the next getline
            cin.ignore(10000, '\n');
            return choice;
        }
        
        cout << "Invalid choice! Please enter q, r, b, or n: ";
    }
}

string ChessGame::generateFEN() const {
    string fen = "";
    
    // 1. Piece placement (from rank 8 to rank 1)
    for (int row = 0; row < 8; row++) {
        int emptyCount = 0;
        
        for (int col = 0; col < 8; col++) {
            int piece = board[row][col];
            
            if (isEmpty(piece)) {
                emptyCount++;
            } else {
                // If there were empty squares, add the count first
                if (emptyCount > 0) {
                    fen += to_string(emptyCount);
                    emptyCount = 0;
                }
                
                // Add the piece character
                char pieceChar = pieceToChar(piece);
                fen += pieceChar;
            }
        }
        
        // Add remaining empty squares count if any
        if (emptyCount > 0) {
            fen += to_string(emptyCount);
        }
        
        // Add rank separator (except after the last rank)
        if (row < 7) {
            fen += '/';
        }
    }
    
    // 2. Active color
    fen += ' ';
    fen += isWhiteTurn ? 'w' : 'b';
    
    // 3. Castling availability
    fen += ' ';
    string castling = "";
    if (!whiteKingMoved) {
        if (!whiteKingsideRookMoved) castling += 'K';
        if (!whiteQueensideRookMoved) castling += 'Q';
    }
    if (!blackKingMoved) {
        if (!blackKingsideRookMoved) castling += 'k';
        if (!blackQueensideRookMoved) castling += 'q';
    }
    fen += castling.empty() ? "-" : castling;
    
    // 4. En passant target square
    fen += ' ';
    if (enPassantTargetRow == -1 || enPassantTargetCol == -1) {
        fen += '-';
    } else {
        fen += coordinateToString(enPassantTargetRow, enPassantTargetCol);
    }
    
    // 5. Halfmove clock (for 50-move rule)
    fen += ' ';
    fen += to_string(halfmoveClock);
    
    // 6. Fullmove number
    fen += ' ';
    fen += to_string(fullmoveNumber);
    
    return fen;
}

void ChessGame::updateFEN() {
    currentFEN = generateFEN();
}