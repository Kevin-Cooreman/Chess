#include "game.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <random>

using namespace std;

// Initialize static Zobrist variables
uint64_t ChessGame::zobristTable[64][12];
uint64_t ChessGame::zobristCastling[16];
uint64_t ChessGame::zobristEnPassant[8];
uint64_t ChessGame::zobristSideToMove;
bool ChessGame::zobristInitialized = false;

ChessGame::ChessGame() {
    initZobrist();  // Initialize Zobrist tables on first construction
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
    positionHistory.clear();
    fenNeedsUpdate = false;  // Will be set by updateFEN()
    updateFEN();
    recordPosition();  // Record starting position
    
    // Compute initial Zobrist hash
    zobristHash = computeZobristHash();
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
        // Clear position history on irreversible moves (captures/pawn moves)
        positionHistory.clear();
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
    
    // Update FEN string and record position
    updateFEN();
    recordPosition();
    
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
        // Clear position history on irreversible moves (captures/pawn moves)
        positionHistory.clear();
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
    
    // Update FEN string and record position
    updateFEN();
    recordPosition();
    
    // Update game status
    updateGameStatus();
    
    cout << "Move: " << moveToString(*matchingMove) << endl;
    
    return true;
}

// Make a move for the engine (saves undo info, doesn't update game status)
void ChessGame::makeMoveForEngine(const Move& move) {
    // Determine captured piece (special case for en passant)
    int capturedPiece;
    if (move.moveType == EN_PASSANT) {
        // For en passant, captured pawn is on the same row as moving pawn
        capturedPiece = board[move.startRow][move.targetColumn];
    } else {
        capturedPiece = board[move.targetRow][move.targetColumn];
    }
    
    // Save current state for undo
    UndoInfo info {
        move,                              // move
        capturedPiece,                     // capturedPiece
        whiteKingMoved,                    // whiteKingMovedBefore
        blackKingMoved,                    // blackKingMovedBefore
        whiteKingsideRookMoved,            // whiteKingsideRookMovedBefore
        whiteQueensideRookMoved,           // whiteQueensideRookMovedBefore
        blackKingsideRookMoved,            // blackKingsideRookMovedBefore
        blackQueensideRookMoved,           // blackQueensideRookMovedBefore
        enPassantTargetRow,                // enPassantTargetRowBefore
        enPassantTargetCol,                // enPassantTargetColBefore
        halfmoveClock,                     // halfmoveClockBefore
        fullmoveNumber,                    // fullmoveNumberBefore
        currentFEN,                        // fenBefore
        zobristHash                        // zobristHashBefore
    };
    
    undoStack.push_back(info);
    
    // === ZOBRIST HASH INCREMENTAL UPDATE ===
    
    // Helper to convert piece to zobrist index
    auto pieceToZobristIndex = [](int piece) -> int {
        bool isWhitePiece = isWhite(piece);
        int pieceValue = piece & 0b0111;
        int typeIndex;
        switch (pieceValue) {
            case 0b001: typeIndex = 0; break;  // PAWN
            case 0b011: typeIndex = 1; break;  // KNIGHT
            case 0b100: typeIndex = 2; break;  // BISHOP
            case 0b010: typeIndex = 3; break;  // ROOK
            case 0b101: typeIndex = 4; break;  // QUEEN
            case 0b110: typeIndex = 5; break;  // KING
            default:     typeIndex = 0; break;
        }
        return isWhitePiece ? typeIndex : (typeIndex + 6);
    };
    
    // Remove old side-to-move bit if it was black's turn
    if (!isWhiteTurn) {
        zobristHash ^= zobristSideToMove;
    }
    
    // Remove old castling rights
    int oldCastlingIndex = 0;
    if (!whiteKingMoved) {
        if (!whiteKingsideRookMoved) oldCastlingIndex |= 1;
        if (!whiteQueensideRookMoved) oldCastlingIndex |= 2;
    }
    if (!blackKingMoved) {
        if (!blackKingsideRookMoved) oldCastlingIndex |= 4;
        if (!blackQueensideRookMoved) oldCastlingIndex |= 8;
    }
    zobristHash ^= zobristCastling[oldCastlingIndex];
    
    // Remove old en passant
    if (enPassantTargetRow != -1 && enPassantTargetCol != -1) {
        zobristHash ^= zobristEnPassant[enPassantTargetCol];
    }
    
    // Remove piece from source square
    int movingPiece = board[move.startRow][move.startColumn];
    int sourceSquare = move.startRow * 8 + move.startColumn;
    zobristHash ^= zobristTable[sourceSquare][pieceToZobristIndex(movingPiece)];
    
    // Remove captured piece if exists
    if (capturedPiece != EMPTY) {
        int captureSquare;
        if (move.moveType == EN_PASSANT) {
            captureSquare = move.startRow * 8 + move.targetColumn;
        } else {
            captureSquare = move.targetRow * 8 + move.targetColumn;
        }
        zobristHash ^= zobristTable[captureSquare][pieceToZobristIndex(capturedPiece)];
    }
    
    // Remove castling rook from old position (BEFORE makeMove)
    if (move.moveType == CASTLING_KINGSIDE) {
        int rookSourceSquare = move.startRow * 8 + 7;
        int rook = board[move.startRow][7];
        zobristHash ^= zobristTable[rookSourceSquare][pieceToZobristIndex(rook)];
    } else if (move.moveType == CASTLING_QUEENSIDE) {
        int rookSourceSquare = move.startRow * 8 + 0;
        int rook = board[move.startRow][0];
        zobristHash ^= zobristTable[rookSourceSquare][pieceToZobristIndex(rook)];
    }
    
    // Execute the move (updates board, castling rights, en passant)
    makeMove(move);
    
    // Add piece to destination square (handles promotion)
    int destSquare = move.targetRow * 8 + move.targetColumn;
    int finalPiece = board[move.targetRow][move.targetColumn];
    zobristHash ^= zobristTable[destSquare][pieceToZobristIndex(finalPiece)];
    
    // Add castling rook to new position (AFTER makeMove)
    if (move.moveType == CASTLING_KINGSIDE) {
        int rookDestSquare = move.startRow * 8 + 5;
        int rook = board[move.startRow][5];
        zobristHash ^= zobristTable[rookDestSquare][pieceToZobristIndex(rook)];
    } else if (move.moveType == CASTLING_QUEENSIDE) {
        int rookDestSquare = move.startRow * 8 + 3;
        int rook = board[move.startRow][3];
        zobristHash ^= zobristTable[rookDestSquare][pieceToZobristIndex(rook)];
    }
    
    // Add new castling rights
    int newCastlingIndex = 0;
    if (!whiteKingMoved) {
        if (!whiteKingsideRookMoved) newCastlingIndex |= 1;
        if (!whiteQueensideRookMoved) newCastlingIndex |= 2;
    }
    if (!blackKingMoved) {
        if (!blackKingsideRookMoved) newCastlingIndex |= 4;
        if (!blackQueensideRookMoved) newCastlingIndex |= 8;
    }
    zobristHash ^= zobristCastling[newCastlingIndex];
    
    // Add new en passant
    if (enPassantTargetRow != -1 && enPassantTargetCol != -1) {
        zobristHash ^= zobristEnPassant[enPassantTargetCol];
    }
    
    // Mark FEN as needing update
    fenNeedsUpdate = true;
    
    // Switch turns
    isWhiteTurn = !isWhiteTurn;
    
    // Add new side-to-move bit if it's now black's turn
    if (!isWhiteTurn) {
        zobristHash ^= zobristSideToMove;
    }
    
    // Increment fullmove number after black's move
    if (isWhiteTurn) {
        fullmoveNumber++;
    }
    
    // Recompute hash (incremental updates have a subtle bug - TODO: fix)
    zobristHash = computeZobristHash();
}

// Make an engine move in the actual game (updates game state properly)
bool ChessGame::makeEngineMove(const Move& move) {
    if (gameOver) {
        cout << "Game is over! " << gameResult << endl;
        return false;
    }
    
    // Verify the move is legal
    vector<Move> legalMoves = getLegalMoves();
    bool isLegal = false;
    for (const Move& legalMove : legalMoves) {
        if (legalMove.startRow == move.startRow &&
            legalMove.startColumn == move.startColumn &&
            legalMove.targetRow == move.targetRow &&
            legalMove.targetColumn == move.targetColumn &&
            legalMove.moveType == move.moveType) {
            isLegal = true;
            break;
        }
    }
    
    if (!isLegal) {
        cout << "Engine attempted illegal move!" << endl;
        return false;
    }
    
    // Check if this move resets the halfmove clock (capture or pawn move)
    int movingPiece = board[move.startRow][move.startColumn];
    int capturedPiece = board[move.targetRow][move.targetColumn];
    bool isPawnMove = (movingPiece & 0b0111) == 0b0001;
    bool isCapture = !isEmpty(capturedPiece) || move.moveType == EN_PASSANT;
    
    if (isPawnMove || isCapture) {
        halfmoveClock = 0;
        // Clear position history on irreversible moves (captures/pawn moves)
        positionHistory.clear();
    } else {
        halfmoveClock++;
    }
    
    // Execute the move
    makeMove(move);
    gameHistory.push_back(move);
    
    // Switch turns
    isWhiteTurn = !isWhiteTurn;
    
    // Increment fullmove number after black's move
    if (isWhiteTurn) {
        fullmoveNumber++;
    }
    
    // Update FEN string and record position
    updateFEN();
    recordPosition();
    
    // Update game status
    updateGameStatus();
    
    
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
    } else if (isDrawByInsufficientMaterial()) {
        gameOver = true;
        gameResult = "Draw by insufficient material!";
    } else if (isDrawByRepetition()) {
        gameOver = true;
        gameResult = "Draw by threefold repetition!";
    } else if (isDrawByFiftyMoveRule()) {
        gameOver = true;
        gameResult = "Draw by fifty-move rule!";
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

// Lazy FEN generation - only regenerate when needed
string ChessGame::getCurrentFEN() const {
    if (fenNeedsUpdate) {
        currentFEN = generateFEN();
        fenNeedsUpdate = false;
    }
    return currentFEN;
}

void ChessGame::updateFEN() {
    currentFEN = generateFEN();
    fenNeedsUpdate = false;
}

string ChessGame::getPositionKey() const {
    // Get FEN without halfmove clock and fullmove number
    // Only the position, active color, castling rights, and en passant matter for repetition
    string fen = getCurrentFEN();  // Use lazy getter
    
    // Find the last two spaces (before halfmove clock and fullmove number)
    size_t lastSpace = fen.rfind(' ');
    if (lastSpace != string::npos) {
        size_t secondLastSpace = fen.rfind(' ', lastSpace - 1);
        if (secondLastSpace != string::npos) {
            return fen.substr(0, secondLastSpace);
        }
    }
    
    return fen;  // Fallback
}

void ChessGame::recordPosition() {
    string posKey = getPositionKey();
    positionHistory[posKey]++;
}

void ChessGame::loadFEN(const string& fen) {
    // Parse FEN string and set up the board
    // FEN format: piece_placement active_color castling en_passant halfmove fullmove
    
    initBoard();  // Clear the board first
    
    istringstream ss(fen);
    string piecePlacement, activeColor, castling, enPassant;
    int halfmove, fullmove;
    
    ss >> piecePlacement >> activeColor >> castling >> enPassant >> halfmove >> fullmove;
    
    // 1. Parse piece placement
    int row = 0, col = 0;
    for (char c : piecePlacement) {
        if (c == '/') {
            row++;
            col = 0;
        } else if (isdigit(c)) {
            col += (c - '0');  // Skip empty squares
        } else {
            board[row][col] = charToPiece(c);
            col++;
        }
    }
    
    // 2. Active color
    isWhiteTurn = (activeColor == "w");
    
    // 3. Castling rights
    whiteKingMoved = (castling.find('K') == string::npos && castling.find('Q') == string::npos);
    blackKingMoved = (castling.find('k') == string::npos && castling.find('q') == string::npos);
    whiteKingsideRookMoved = (castling.find('K') == string::npos);
    whiteQueensideRookMoved = (castling.find('Q') == string::npos);
    blackKingsideRookMoved = (castling.find('k') == string::npos);
    blackQueensideRookMoved = (castling.find('q') == string::npos);
    
    // 4. En passant target square
    if (enPassant != "-") {
        int epRow, epCol;
        if (parseCoordinate(enPassant, epRow, epCol)) {
            // En passant target square is where the pawn would land
            // We need to set the target behind the pawn that just moved
            if (epRow == 2) {  // White pawn moved from row 6 to row 4
                enPassantTargetRow = 3;
                enPassantTargetCol = epCol;
            } else if (epRow == 5) {  // Black pawn moved from row 1 to row 3
                enPassantTargetRow = 4;
                enPassantTargetCol = epCol;
            }
        }
    } else {
        enPassantTargetRow = -1;
        enPassantTargetCol = -1;
    }
    
    // 5. Halfmove clock and fullmove number
    halfmoveClock = halfmove;
    fullmoveNumber = fullmove;
    
    // Reset game state
    gameOver = false;
    gameResult = "";
    gameHistory.clear();
    positionHistory.clear();
    
    // Update FEN and record position
    updateFEN();
    recordPosition();
    
    // Recompute zobrist hash from new position
    zobristHash = computeZobristHash();
}

bool ChessGame::isDrawByRepetition() const {
    string posKey = getPositionKey();
    
    // Check if this position has occurred 3 or more times
    auto it = positionHistory.find(posKey);
    if (it != positionHistory.end() && it->second >= 3) {
        return true;
    }
    
    return false;
}

bool ChessGame::isDrawByFiftyMoveRule() const {
    // 50-move rule: 50 moves (100 half-moves) without capture or pawn move
    return halfmoveClock >= 100;
}

bool ChessGame::isDrawByInsufficientMaterial() const {
    // Count pieces on the board
    int whiteBishops = 0, blackBishops = 0;
    int whiteKnights = 0, blackKnights = 0;
    int whitePawns = 0, blackPawns = 0;
    int whiteRooks = 0, blackRooks = 0;
    int whiteQueens = 0, blackQueens = 0;
    
    // Track bishop square colors (true = light square, false = dark square)
    bool whiteBishopOnLight = false, whiteBishopOnDark = false;
    bool blackBishopOnLight = false, blackBishopOnDark = false;
    
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            int piece = board[row][col];
            if (isEmpty(piece)) continue;
            
            int pieceType = piece & 0b0111;
            bool isLightSquare = (row + col) % 2 == 0;
            
            if (isWhite(piece)) {
                switch (pieceType) {
                    case 0b0001: whitePawns++; break;
                    case 0b0010: whiteRooks++; break;
                    case 0b0011: whiteKnights++; break;
                    case 0b0100: 
                        whiteBishops++; 
                        if (isLightSquare) whiteBishopOnLight = true;
                        else whiteBishopOnDark = true;
                        break;
                    case 0b0101: whiteQueens++; break;
                }
            } else {
                switch (pieceType) {
                    case 0b0001: blackPawns++; break;
                    case 0b0010: blackRooks++; break;
                    case 0b0011: blackKnights++; break;
                    case 0b0100: 
                        blackBishops++; 
                        if (isLightSquare) blackBishopOnLight = true;
                        else blackBishopOnDark = true;
                        break;
                    case 0b0101: blackQueens++; break;
                }
            }
        }
    }
    
    // If there are pawns, rooks, or queens, checkmate is possible
    if (whitePawns > 0 || blackPawns > 0 || 
        whiteRooks > 0 || blackRooks > 0 || 
        whiteQueens > 0 || blackQueens > 0) {
        return false;
    }
    
    // King vs King
    if (whiteKnights == 0 && blackKnights == 0 && 
        whiteBishops == 0 && blackBishops == 0) {
        return true;
    }
    
    // King + Bishop vs King
    if (whiteKnights == 0 && blackKnights == 0 &&
        ((whiteBishops == 1 && blackBishops == 0) ||
         (whiteBishops == 0 && blackBishops == 1))) {
        return true;
    }
    
    // King + Knight vs King
    if (whiteBishops == 0 && blackBishops == 0 &&
        ((whiteKnights == 1 && blackKnights == 0) ||
         (whiteKnights == 0 && blackKnights == 1))) {
        return true;
    }
    
    // King + Bishop vs King + Bishop (same color bishops)
    if (whiteKnights == 0 && blackKnights == 0 &&
        whiteBishops == 1 && blackBishops == 1) {
        // Check if both bishops are on the same color squares
        if ((whiteBishopOnLight && blackBishopOnLight) ||
            (whiteBishopOnDark && blackBishopOnDark)) {
            return true;
        }
    }
    
    return false;
}

// Undo the last move - used by the engine for minimax search
void ChessGame::undoMove() {
    if (undoStack.empty()) {
        return;
    }
    
    // Pop the last undo info
    UndoInfo info = undoStack.back();
    undoStack.pop_back();
    
    // Restore game state flags
    whiteKingMoved = info.whiteKingMovedBefore;
    blackKingMoved = info.blackKingMovedBefore;
    whiteKingsideRookMoved = info.whiteKingsideRookMovedBefore;
    whiteQueensideRookMoved = info.whiteQueensideRookMovedBefore;
    blackKingsideRookMoved = info.blackKingsideRookMovedBefore;
    blackQueensideRookMoved = info.blackQueensideRookMovedBefore;
    enPassantTargetRow = info.enPassantTargetRowBefore;
    enPassantTargetCol = info.enPassantTargetColBefore;
    halfmoveClock = info.halfmoveClockBefore;
    fullmoveNumber = info.fullmoveNumberBefore;
    currentFEN = info.fenBefore;
    fenNeedsUpdate = false;  // FEN is now current (restored from undo)
    
    // Restore zobrist hash (much faster than recalculating)
    zobristHash = info.zobristHashBefore;
    
    // Undo the move on the board
    Move& move = info.move;
    int movingPiece = board[move.targetRow][move.targetColumn];
    
    // Handle special move types
    switch(move.moveType) {
        case CASTLING_KINGSIDE:
            // Move king back
            board[move.startRow][move.startColumn] = movingPiece;
            board[move.targetRow][move.targetColumn] = EMPTY;
            // Move rook back
            board[move.targetRow][7] = board[move.targetRow][5];
            board[move.targetRow][5] = EMPTY;
            break;
            
        case CASTLING_QUEENSIDE:
            // Move king back
            board[move.startRow][move.startColumn] = movingPiece;
            board[move.targetRow][move.targetColumn] = EMPTY;
            // Move rook back
            board[move.targetRow][0] = board[move.targetRow][3];
            board[move.targetRow][3] = EMPTY;
            break;
            
        case EN_PASSANT:
            // Move pawn back
            board[move.startRow][move.startColumn] = movingPiece;
            board[move.targetRow][move.targetColumn] = EMPTY;
            // Restore captured pawn (it was on the same rank as the moving pawn)
            board[move.startRow][move.targetColumn] = info.capturedPiece;
            break;
            
        case PAWN_PROMOTION:
            // Convert promoted piece back to pawn
            if (isWhite(movingPiece)) {
                board[move.startRow][move.startColumn] = WHITE_PAWN;
            } else {
                board[move.startRow][move.startColumn] = BLACK_PAWN;
            }
            board[move.targetRow][move.targetColumn] = info.capturedPiece;
            break;
            
        default: // NORMAL move
            board[move.startRow][move.startColumn] = movingPiece;
            board[move.targetRow][move.targetColumn] = info.capturedPiece;
            break;
    }
    
    // Switch turn back
    isWhiteTurn = !isWhiteTurn;
    
    // Remove from game history if it was added
    if (!gameHistory.empty() && 
        gameHistory.back().startRow == move.startRow &&
        gameHistory.back().startColumn == move.startColumn &&
        gameHistory.back().targetRow == move.targetRow &&
        gameHistory.back().targetColumn == move.targetColumn) {
        gameHistory.pop_back();
    }
}

// Clear the undo stack (used after engine search completes)
void ChessGame::clearUndoStack() {
    undoStack.clear();
}

// Make a null move (pass turn) for null move pruning
void ChessGame::makeNullMove() {
    // Save old en passant for undo
    nullMoveOldEnPassantRow = enPassantTargetRow;
    nullMoveOldEnPassantCol = enPassantTargetCol;
    
    // Remove old en passant from hash
    if (enPassantTargetRow != -1 && enPassantTargetCol != -1) {
        zobristHash ^= zobristEnPassant[enPassantTargetCol];
    }
    
    // Simply switch the turn - no pieces move
    isWhiteTurn = !isWhiteTurn;
    
    // Toggle side to move in hash
    zobristHash ^= zobristSideToMove;
    
    // Reset en passant (can't en passant after null move)
    enPassantTargetRow = -1;
    enPassantTargetCol = -1;
    
    // Mark FEN as needing update
    fenNeedsUpdate = true;
}

// Undo a null move
void ChessGame::undoNullMove() {
    // Switch turn back
    isWhiteTurn = !isWhiteTurn;
    
    // Toggle side to move back in hash
    zobristHash ^= zobristSideToMove;
    
    // Restore en passant state
    enPassantTargetRow = nullMoveOldEnPassantRow;
    enPassantTargetCol = nullMoveOldEnPassantCol;
    
    // Add restored en passant back to hash
    if (enPassantTargetRow != -1 && enPassantTargetCol != -1) {
        zobristHash ^= zobristEnPassant[enPassantTargetCol];
    }
    
    fenNeedsUpdate = true;
}// Zobrist hashing functions for ChessGame
// These are added to the end of game.cpp

// Initialize Zobrist random number tables
void ChessGame::initZobrist() {
    if (zobristInitialized) return;  // Only initialize once
    
    random_device rd;
    mt19937_64 gen(rd());
    uniform_int_distribution<uint64_t> dist;
    
    // Generate random numbers for each piece on each square
    // Piece encoding: 0-5 = white pieces (pawn, knight, bishop, rook, queen, king)
    //                 6-11 = black pieces (pawn, knight, bishop, rook, queen, king)
    for (int square = 0; square < 64; square++) {
        for (int piece = 0; piece < 12; piece++) {
            zobristTable[square][piece] = dist(gen);
        }
    }
    
    // Generate random numbers for castling rights (16 combinations: 2^4)
    for (int i = 0; i < 16; i++) {
        zobristCastling[i] = dist(gen);
    }
    
    // Generate random numbers for en passant file (8 files: a-h)
    for (int i = 0; i < 8; i++) {
        zobristEnPassant[i] = dist(gen);
    }
    
    // Generate random number for side to move
    zobristSideToMove = dist(gen);
    
    zobristInitialized = true;
}

// Compute Zobrist hash from current board state
uint64_t ChessGame::computeZobristHash() const {
    uint64_t hash = 0;
    
    // XOR all pieces on the board
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            int piece = board[row][col];
            if (piece != EMPTY) {
                // Convert board piece encoding to zobrist index (0-11)
                bool isWhitePiece = isWhite(piece);
                int pieceValue = piece & 0b0111;  // Get piece type (lower 3 bits)
                
                // Map piece type to index 0-5
                int typeIndex;
                switch (pieceValue) {
                    case 0b001: typeIndex = 0; break;  // PAWN
                    case 0b011: typeIndex = 1; break;  // KNIGHT
                    case 0b100: typeIndex = 2; break;  // BISHOP
                    case 0b010: typeIndex = 3; break;  // ROOK
                    case 0b101: typeIndex = 4; break;  // QUEEN
                    case 0b110: typeIndex = 5; break;  // KING
                    default:     typeIndex = 0; break;
                }
                
                // White pieces: 0-5, Black pieces: 6-11
                int zobristIndex = isWhitePiece ? typeIndex : (typeIndex + 6);
                int square = row * 8 + col;
                
                hash ^= zobristTable[square][zobristIndex];
            }
        }
    }
    
    // XOR castling rights
    int castlingIndex = 0;
    if (!whiteKingMoved) {
        if (!whiteKingsideRookMoved) castlingIndex |= 1;   // White kingside
        if (!whiteQueensideRookMoved) castlingIndex |= 2;  // White queenside
    }
    if (!blackKingMoved) {
        if (!blackKingsideRookMoved) castlingIndex |= 4;   // Black kingside
        if (!blackQueensideRookMoved) castlingIndex |= 8;  // Black queenside
    }
    hash ^= zobristCastling[castlingIndex];
    
    // XOR en passant file if exists
    if (enPassantTargetRow != -1 && enPassantTargetCol != -1) {
        hash ^= zobristEnPassant[enPassantTargetCol];
    }
    
    // XOR side to move (only if black to move)
    if (!isWhiteTurn) {
        hash ^= zobristSideToMove;
    }
    
    return hash;
}
