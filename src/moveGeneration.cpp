#include "moveGeneration.hpp"
#include <iostream>

using namespace std;

vector<Move> generateUpMoves(int sRow, int sCol){
    vector<Move> moves;

    //up
    for(int targetRow = sRow -1; targetRow >= 0; targetRow--){

        if(sameColour(board[sRow][sCol], board[targetRow][sCol])){
            break;
        }

        else if(!sameColour(board[sRow][sCol], board[targetRow][sCol]) && !isEmpty(board[targetRow][sCol])){
            moves.push_back(Move(sRow, sCol, targetRow, sCol));
            break;
        }

        else{
            moves.push_back(Move(sRow, sCol, targetRow, sCol));
        }
    }

    return moves;
}

vector<Move> generateDownMoves(int sRow, int sCol){
    vector<Move> moves;

    for(int targetRow = sRow +1; targetRow < 8; targetRow++){

        if(sameColour(board[sRow][sCol], board[targetRow][sCol])){
            break;
        }

        else if(!sameColour(board[sRow][sCol], board[targetRow][sCol]) && !isEmpty(board[targetRow][sCol])){
            moves.push_back(Move(sRow, sCol, targetRow, sCol));
            break;
        }

        else{
            moves.push_back(Move(sRow, sCol, targetRow, sCol));
        }
    }
    return moves;
}

vector<Move> generateleftMoves(int sRow, int sCol){
    vector<Move> moves;
    for(int targetCol = sCol -1; targetCol >= 0; targetCol--){

        if(sameColour(board[sRow][sCol], board[sRow][targetCol])){
            break;
        }

        else if(!sameColour(board[sRow][sCol], board[sRow][targetCol]) && !isEmpty(board[sRow][targetCol])){
            moves.push_back(Move(sRow, sCol, sRow, targetCol));
            break;
        }

        else{
            moves.push_back(Move(sRow, sCol, sRow, targetCol));
        }
    }
    return moves;
}
vector<Move> generateRightMoves(int sRow, int sCol){
    vector<Move> moves;
    for(int targetCol = sCol +1; targetCol < 8; targetCol++){

        if(sameColour(board[sRow][sCol], board[sRow][targetCol])){
            break;
        }

        else if(!sameColour(board[sRow][sCol], board[sRow][targetCol]) && !isEmpty(board[sRow][targetCol])){
            moves.push_back(Move(sRow, sCol, sRow, targetCol));
            break;
        }

        else{
            moves.push_back(Move(sRow, sCol, sRow, targetCol));
        }
    }
    return moves;
}

vector<Move> generateUpLeftMoves(int sRow, int sCol){
    vector<Move> moves;

    for(int i = 1; i < 8; i++){
        int targetRow = sRow - i;  // up
        int targetCol = sCol - i;  // left
        
        if(targetRow < 0 || targetCol < 0) break; // Off board
        
        if(sameColour(board[sRow][sCol], board[targetRow][targetCol])){
            break;
        }
        else if(!sameColour(board[sRow][sCol], board[targetRow][targetCol]) && !isEmpty(board[targetRow][targetCol])){
            moves.push_back(Move(sRow, sCol, targetRow, targetCol));
            break;
        }
        else{
            moves.push_back(Move(sRow, sCol, targetRow, targetCol));
        }
    }
    return moves;
}

vector<Move> generateDownLeftMoves(int sRow, int sCol){
    vector<Move> moves;

    for(int i = 1; i < 8; i++){
        int targetRow = sRow + i;  // down
        int targetCol = sCol - i;  // left
        
        if(targetRow > 7 || targetCol < 0) break; // Off board
        
        if(sameColour(board[sRow][sCol], board[targetRow][targetCol])){
            break;
        }
        else if(!sameColour(board[sRow][sCol], board[targetRow][targetCol]) && !isEmpty(board[targetRow][targetCol])){
            moves.push_back(Move(sRow, sCol, targetRow, targetCol));
            break;
        }
        else{
            moves.push_back(Move(sRow, sCol, targetRow, targetCol));
        }
    }

    return moves;
}
vector<Move> generateUpRightMoves(int sRow, int sCol){
    vector<Move> moves;

    for(int i = 1; i < 8; i++){
        int targetRow = sRow - i;  // up
        int targetCol = sCol + i;  // right
        
        if(targetRow < 0 || targetCol > 7) break; // Off board
        
        if(sameColour(board[sRow][sCol], board[targetRow][targetCol])){
            break;
        }
        else if(!sameColour(board[sRow][sCol], board[targetRow][targetCol]) && !isEmpty(board[targetRow][targetCol])){
            moves.push_back(Move(sRow, sCol, targetRow, targetCol));
            break;
        }
        else{
            moves.push_back(Move(sRow, sCol, targetRow, targetCol));
        }
    }

    return moves;
}

vector<Move> generateDownRightMoves(int sRow, int sCol){
    vector<Move> moves;

    for(int i = 1; i < 8; i++){
        int targetRow = sRow + i;  // down
        int targetCol = sCol + i;  // right
        
        if(targetRow > 7 || targetCol > 7) break; // Off board
        
        if(sameColour(board[sRow][sCol], board[targetRow][targetCol])){
            break;
        }
        else if(!sameColour(board[sRow][sCol], board[targetRow][targetCol]) && !isEmpty(board[targetRow][targetCol])){
            moves.push_back(Move(sRow, sCol, targetRow, targetCol));
            break;
        }
        else{
            moves.push_back(Move(sRow, sCol, targetRow, targetCol));
        }
    }

    return moves;
}

vector<Move> generateRookMoves(int sRow, int sCol){
    vector<Move> moves;
    
    // Use helper functions for each direction
    vector<Move> upMoves = generateUpMoves(sRow, sCol);
    vector<Move> downMoves = generateDownMoves(sRow, sCol);
    vector<Move> leftMoves = generateleftMoves(sRow, sCol);
    vector<Move> rightMoves = generateRightMoves(sRow, sCol);
    
    // Combine all moves
    moves.insert(moves.end(), upMoves.begin(), upMoves.end());
    moves.insert(moves.end(), downMoves.begin(), downMoves.end());
    moves.insert(moves.end(), leftMoves.begin(), leftMoves.end());
    moves.insert(moves.end(), rightMoves.begin(), rightMoves.end());
    
    return moves;
}

vector<Move> generateBishopMoves(int sRow, int sCol){
    vector<Move> moves;
    
    // Use helper functions for each diagonal direction
    vector<Move> upLeftMoves = generateUpLeftMoves(sRow, sCol);
    vector<Move> upRightMoves = generateUpRightMoves(sRow, sCol);
    vector<Move> downLeftMoves = generateDownLeftMoves(sRow, sCol);
    vector<Move> downRightMoves = generateDownRightMoves(sRow, sCol);
    
    // Combine all moves
    moves.insert(moves.end(), upLeftMoves.begin(), upLeftMoves.end());
    moves.insert(moves.end(), upRightMoves.begin(), upRightMoves.end());
    moves.insert(moves.end(), downLeftMoves.begin(), downLeftMoves.end());
    moves.insert(moves.end(), downRightMoves.begin(), downRightMoves.end());
    
    return moves;
}

vector<Move> generateQueenMoves(int sRow, int sCol){
    vector<Move> moves = generateRookMoves(sRow, sCol);  // Start with rook moves
    
    vector<Move> bishopMoves = generateBishopMoves(sRow, sCol);
    moves.insert(moves.end(), bishopMoves.begin(), bishopMoves.end());  // Add bishop moves
    
    return moves;
}

vector<Move> generateKingMoves(int sRow, int sCol){
    vector<Move> moves;
    
    // All 8 possible directions (row offset, col offset)
    int directions[8][2] = {
        {-1, -1}, // Up-Left
        {-1,  0}, // Up
        {-1,  1}, // Up-Right
        { 0, -1}, // Left
        { 0,  1}, // Right
        { 1, -1}, // Down-Left
        { 1,  0}, // Down
        { 1,  1}  // Down-Right
    };
    
    for(int i = 0; i < 8; i++){
        int targetRow = sRow + directions[i][0];
        int targetCol = sCol + directions[i][1];
        
        // Check if target is on board
        if(targetRow >= 0 && targetRow < 8 && targetCol >= 0 && targetCol < 8){
            
            // Same logic as other pieces:
            if(sameColour(board[sRow][sCol], board[targetRow][targetCol])){
                // Same color piece - can't move there
                continue; // Skip this move
            }
            else {
                // Empty square or enemy piece - can move there
                moves.push_back(Move(sRow, sCol, targetRow, targetCol));
            }
        }
    }
    
    // Add castling moves
    bool isWhitePiece = isWhite(board[sRow][sCol]);
    vector<Move> castlingMoves = generateCastlingMoves(isWhitePiece);
    moves.insert(moves.end(), castlingMoves.begin(), castlingMoves.end());
    
    return moves;
}

vector<Move> generateKnightMoves(int sRow, int sCol){
    vector<Move> moves;
    
    // All 8 possible directions (row offset, col offset)
    int directions[8][2] = {
        {-1, -2}, // Up-Left-Left
        {-2, -1}, // Up-Up-Left
        {-2, 1}, // Up-Up-Right
        {-1, 2}, // Up-Right-Right 
        { 1, 2}, // Down-Right-Right 
        { 2, 1}, // Down-Down-Right 
        { 2, -1}, // Down-Down-Left 
        { 1, -2}  // Down-Left-Left 
    };
    
    for(int i = 0; i < 8; i++){
        int targetRow = sRow + directions[i][0];
        int targetCol = sCol + directions[i][1];
        
        // Check if target is on board
        if(targetRow >= 0 && targetRow < 8 && targetCol >= 0 && targetCol < 8){
            
            // Same logic as other pieces:
            if(sameColour(board[sRow][sCol], board[targetRow][targetCol])){
                // Same color piece - can't move there
                continue; // Skip this move
            }
            else {
                // Empty square or enemy piece - can move there
                moves.push_back(Move(sRow, sCol, targetRow, targetCol));
            }
        }
    }
    
    return moves;
}

vector<Move> generatePawnMoves(int sRow, int sCol){

    vector<Move> moves;

    //logic white pawn
    if(isWhite(board[sRow][sCol])){

        //pawn on first rank
        //4 possible moves
        if(sRow == 6){
            //basic move structure
            for(int i = -1; i < 2; i++){
                //just 1 up
                int targetRow = sRow - 1;
                int targetCol = sCol + i;

                //take diagonally
                if(isBlack(board[targetRow][targetCol]) && targetCol != sCol){
                    // Check for promotion
                    if(targetRow == 0) {
                        vector<Move> promotionMoves = generatePawnPromotionMoves(sRow, sCol, targetRow, targetCol);
                        moves.insert(moves.end(), promotionMoves.begin(), promotionMoves.end());
                    } else {
                        moves.push_back(Move(sRow, sCol, targetRow, targetCol));
                    }
                }
                //or move up 1 or 2 up on start square
                else if(isEmpty(board[targetRow][targetCol]) &&  targetCol == sCol){
                    // Check for promotion
                    if(targetRow == 0) {
                        vector<Move> promotionMoves = generatePawnPromotionMoves(sRow, sCol, targetRow, targetCol);
                        moves.insert(moves.end(), promotionMoves.begin(), promotionMoves.end());
                    } else {
                        moves.push_back(Move(sRow, sCol, targetRow, targetCol));
                    }
                    // Two-square move from starting position
                    if(sRow == 6 && isEmpty(board[targetRow-1][targetCol])) {
                        moves.push_back(Move(sRow, sCol, targetRow-1, targetCol));
                    }
                }
            }   
        }

        else{
            //basic move structure
            for(int i = -1; i < 2; i++){
                //just 1 up
                int targetRow = sRow - 1;
                int targetCol = sCol + i;

                // Check bounds
                if(targetRow < 0 || targetCol < 0 || targetCol > 7) continue;

                //take diagonally
                if(isBlack(board[targetRow][targetCol]) && targetCol != sCol){
                    // Check for promotion
                    if(targetRow == 0) {
                        vector<Move> promotionMoves = generatePawnPromotionMoves(sRow, sCol, targetRow, targetCol);
                        moves.insert(moves.end(), promotionMoves.begin(), promotionMoves.end());
                    } else {
                        moves.push_back(Move(sRow, sCol, targetRow, targetCol));
                    }
                }
                //or move up 1 square
                else if(isEmpty(board[targetRow][targetCol]) &&  targetCol == sCol){
                    // Check for promotion
                    if(targetRow == 0) {
                        vector<Move> promotionMoves = generatePawnPromotionMoves(sRow, sCol, targetRow, targetCol);
                        moves.insert(moves.end(), promotionMoves.begin(), promotionMoves.end());
                    } else {
                        moves.push_back(Move(sRow, sCol, targetRow, targetCol));
                    }
                }
            }
            
            // Add en passant moves for white pawns
            vector<Move> enPassantMoves = generateEnPassantMoves(sRow, sCol);
            moves.insert(moves.end(), enPassantMoves.begin(), enPassantMoves.end());
        }
    }

    else{
         //pawn on first rank
        //4 possible moves
        if(sRow == 1){
            //basic move structure
            for(int i = -1; i < 2; i++){
                //just 1 down
                int targetRow = sRow + 1;
                int targetCol = sCol + i;

                // Check bounds
                if(targetRow > 7 || targetCol < 0 || targetCol > 7) continue;

                //take diagonally
                if(isWhite(board[targetRow][targetCol]) && targetCol != sCol){
                    // Check for promotion
                    if(targetRow == 7) {
                        vector<Move> promotionMoves = generatePawnPromotionMoves(sRow, sCol, targetRow, targetCol);
                        moves.insert(moves.end(), promotionMoves.begin(), promotionMoves.end());
                    } else {
                        moves.push_back(Move(sRow, sCol, targetRow, targetCol));
                    }
                }
                //or move down 1 or 2 down from start square
                else if(isEmpty(board[targetRow][targetCol]) &&  targetCol == sCol){
                    // Check for promotion
                    if(targetRow == 7) {
                        vector<Move> promotionMoves = generatePawnPromotionMoves(sRow, sCol, targetRow, targetCol);
                        moves.insert(moves.end(), promotionMoves.begin(), promotionMoves.end());
                    } else {
                        moves.push_back(Move(sRow, sCol, targetRow, targetCol));
                    }
                    // Two-square move from starting position
                    if(sRow == 1 && isEmpty(board[targetRow+1][targetCol])) {
                        moves.push_back(Move(sRow, sCol, targetRow+1, targetCol));
                    }
                }
            }   
        }

        else{
            //basic move structure
            for(int i = -1; i < 2; i++){
                //just 1 down
                int targetRow = sRow + 1;
                int targetCol = sCol + i;

                // Check bounds
                if(targetRow > 7 || targetCol < 0 || targetCol > 7) continue;

                //take diagonally
                if(isWhite(board[targetRow][targetCol]) && targetCol != sCol){
                    // Check for promotion
                    if(targetRow == 7) {
                        vector<Move> promotionMoves = generatePawnPromotionMoves(sRow, sCol, targetRow, targetCol);
                        moves.insert(moves.end(), promotionMoves.begin(), promotionMoves.end());
                    } else {
                        moves.push_back(Move(sRow, sCol, targetRow, targetCol));
                    }
                }
                //or move down 1 square
                else if(isEmpty(board[targetRow][targetCol]) &&  targetCol == sCol){
                    // Check for promotion
                    if(targetRow == 7) {
                        vector<Move> promotionMoves = generatePawnPromotionMoves(sRow, sCol, targetRow, targetCol);
                        moves.insert(moves.end(), promotionMoves.begin(), promotionMoves.end());
                    } else {
                        moves.push_back(Move(sRow, sCol, targetRow, targetCol));
                    }
                }
            }
            
            // Add en passant moves for black pawns
            vector<Move> enPassantMoves = generateEnPassantMoves(sRow, sCol);
            moves.insert(moves.end(), enPassantMoves.begin(), enPassantMoves.end());
        }
    }
    
    return moves;
}

// Generate basic moves without special moves (used for attack detection to avoid infinite recursion)
vector<Move> generateBasicMovesForPiece(int row, int col) {
    int piece = board[row][col];
    int pieceType = piece & 0b0111; // Extract piece type
    
    switch(pieceType) {
        case 0b0001: { // Pawn - basic moves only (no en passant, no promotion)
            vector<Move> moves;
            bool isWhitePawn = isWhite(piece);
            int direction = isWhitePawn ? -1 : 1;
            int targetRow = row + direction;
            
            // Forward move
            if (targetRow >= 0 && targetRow < 8 && isEmpty(board[targetRow][col])) {
                moves.push_back(Move(row, col, targetRow, col));
                
                // Double move from starting position
                if ((isWhitePawn && row == 6) || (!isWhitePawn && row == 1)) {
                    targetRow = row + 2 * direction;
                    if (targetRow >= 0 && targetRow < 8 && isEmpty(board[targetRow][col])) {
                        moves.push_back(Move(row, col, targetRow, col));
                    }
                }
            }
            
            // Diagonal captures
            for (int colOffset = -1; colOffset <= 1; colOffset += 2) {
                int targetCol = col + colOffset;
                if (targetCol >= 0 && targetCol < 8 && targetRow >= 0 && targetRow < 8) {
                    int targetPiece = board[targetRow][targetCol];
                    if (!isEmpty(targetPiece) && isWhite(targetPiece) != isWhitePawn) {
                        moves.push_back(Move(row, col, targetRow, targetCol));
                    }
                }
            }
            return moves;
        }
        case 0b0010: return generateRookMoves(row, col);
        case 0b0011: return generateKnightMoves(row, col);
        case 0b0100: return generateBishopMoves(row, col);
        case 0b0101: return generateQueenMoves(row, col);
        case 0b0110: { // King - basic moves only (no castling)
            vector<Move> moves;
            int directions[8][2] = {
                {-1, -1}, {-1,  0}, {-1,  1}, { 0, -1},
                { 0,  1}, { 1, -1}, { 1,  0}, { 1,  1}
            };
            
            for(int i = 0; i < 8; i++){
                int targetRow = row + directions[i][0];
                int targetCol = col + directions[i][1];
                
                if(targetRow >= 0 && targetRow < 8 && targetCol >= 0 && targetCol < 8){
                    if(!sameColour(board[row][col], board[targetRow][targetCol])){
                        moves.push_back(Move(row, col, targetRow, targetCol));
                    }
                }
            }
            return moves;
        }
        default: return vector<Move>(); // Empty vector for invalid pieces
    }
}

// This replaces the need for separate piece-type checking
vector<Move> generateMovesForPiece(int row, int col) {
    int piece = board[row][col];
    int pieceType = piece & 0b0111; // Extract piece type
    
    switch(pieceType) {
        case 0b0001: return generatePawnMoves(row, col);
        case 0b0010: return generateRookMoves(row, col);
        case 0b0011: return generateKnightMoves(row, col);
        case 0b0100: return generateBishopMoves(row, col);
        case 0b0101: return generateQueenMoves(row, col);
        case 0b0110: return generateKingMoves(row, col);
        default: return vector<Move>(); // Empty vector for invalid pieces
    }
}

bool isSquareAttacked(int row, int col, bool colour) {
    // Check if any enemy piece of the specified color can attack this square
    
    for(int r = 0; r < 8; r++) {
        for(int c = 0; c < 8; c++) {
            int piece = board[r][c];
            
            // Skip empty squares
            if(isEmpty(piece)) continue;
            
            // Check if this piece belongs to the attacking color
            if(isWhite(piece) != colour) continue;
            
            // Generate basic moves for this piece (no special moves to avoid infinite recursion)
            vector<Move> pieceMoves = generateBasicMovesForPiece(r, c);
            
            for(const Move& move : pieceMoves) {
                if(move.targetRow == row && move.targetColumn == col) {
                    return true; // This square is attacked!
                }
            }
        }
    }
    
    return false; // Square is safe
}

bool isKingInCheck(bool whiteKing) {
    int kingPiece = whiteKing ? WHITE_KING : BLACK_KING;
    
    // Find the king position
    for(int row = 0; row < 8; row++) {
        for(int col = 0; col < 8; col++) {
            if(board[row][col] == kingPiece) {
                // Found the king, check if enemy can attack this square
                return isSquareAttacked(row, col, !whiteKing);
            }
        }
    }
    
    return false; // King not found (shouldn't happen in valid game)
}

bool isMoveLegal(const Move& move) {
    // Get the pieces involved
    int movingPiece = board[move.startRow][move.startColumn];
    int capturedPiece = board[move.targetRow][move.targetColumn];
    
    // Temporarily make the move
    board[move.targetRow][move.targetColumn] = movingPiece;
    board[move.startRow][move.startColumn] = EMPTY;
    
    // Check if our king would be in check after this move
    bool wouldBeInCheck = isKingInCheck(isWhite(movingPiece));
    
    // Undo the move (restore original position)
    board[move.startRow][move.startColumn] = movingPiece;
    board[move.targetRow][move.targetColumn] = capturedPiece;
    
    // Move is legal if king is NOT in check
    return !wouldBeInCheck;
}

vector<Move> generateLegalMoves(bool isWhiteTurn) {
    vector<Move> legalMoves;
    
    // Scan entire board
    for(int row = 0; row < 8; row++) {
        for(int col = 0; col < 8; col++) {
            int piece = board[row][col];
            
            // Skip empty squares and opponent pieces
            if(isEmpty(piece) || isWhite(piece) != isWhiteTurn) continue;
            
            // Generate pseudo-legal moves for this piece
            vector<Move> pieceMoves = generateMovesForPiece(row, col);
            
            // Filter to only legal moves
            for(const Move& move : pieceMoves) {
                if(isMoveLegal(move)) {
                    legalMoves.push_back(move);
                }
            }
        }
    }
    
    return legalMoves;
}

// Castling move generation
vector<Move> generateCastlingMoves(bool isWhite) {
    vector<Move> moves;
    
    int kingRow = isWhite ? 7 : 0;
    int king = isWhite ? WHITE_KING : BLACK_KING;
    int rook = isWhite ? WHITE_ROOK : BLACK_ROOK;
    
    // Check if king is in starting position and hasn't moved
    if(board[kingRow][4] != king) return moves;
    if(isWhite && whiteKingMoved) return moves;
    if(!isWhite && blackKingMoved) return moves;
    
    // King can't castle while in check
    if(isKingInCheck(isWhite)) return moves;
    
    // Kingside castling
    if(board[kingRow][7] == rook) { // Rook is there
        if(isWhite && !whiteKingsideRookMoved) {
            // Check squares between king and rook are empty
            if(isEmpty(board[kingRow][5]) && isEmpty(board[kingRow][6])) {
                // Check king doesn't pass through or land on attacked square
                if(!isSquareAttacked(kingRow, 5, !isWhite) && !isSquareAttacked(kingRow, 6, !isWhite)) {
                    moves.push_back(Move(kingRow, 4, kingRow, 6, CASTLING_KINGSIDE));
                }
            }
        }
        if(!isWhite && !blackKingsideRookMoved) {
            // Check squares between king and rook are empty
            if(isEmpty(board[kingRow][5]) && isEmpty(board[kingRow][6])) {
                // Check king doesn't pass through or land on attacked square
                if(!isSquareAttacked(kingRow, 5, !isWhite) && !isSquareAttacked(kingRow, 6, !isWhite)) {
                    moves.push_back(Move(kingRow, 4, kingRow, 6, CASTLING_KINGSIDE));
                }
            }
        }
    }
    
    // Queenside castling
    if(board[kingRow][0] == rook) { // Rook is there
        if(isWhite && !whiteQueensideRookMoved) {
            // Check squares between king and rook are empty
            if(isEmpty(board[kingRow][1]) && isEmpty(board[kingRow][2]) && isEmpty(board[kingRow][3])) {
                // Check king doesn't pass through or land on attacked square
                if(!isSquareAttacked(kingRow, 2, !isWhite) && !isSquareAttacked(kingRow, 3, !isWhite)) {
                    moves.push_back(Move(kingRow, 4, kingRow, 2, CASTLING_QUEENSIDE));
                }
            }
        }
        if(!isWhite && !blackQueensideRookMoved) {
            // Check squares between king and rook are empty
            if(isEmpty(board[kingRow][1]) && isEmpty(board[kingRow][2]) && isEmpty(board[kingRow][3])) {
                // Check king doesn't pass through or land on attacked square
                if(!isSquareAttacked(kingRow, 2, !isWhite) && !isSquareAttacked(kingRow, 3, !isWhite)) {
                    moves.push_back(Move(kingRow, 4, kingRow, 2, CASTLING_QUEENSIDE));
                }
            }
        }
    }
    
    return moves;
}

// En passant move generation
vector<Move> generateEnPassantMoves(int sRow, int sCol) {
    vector<Move> moves;
    
    // Only pawns can do en passant
    int piece = board[sRow][sCol];
    if((piece & 0b0111) != 0b0001) return moves; // Not a pawn
    
    // Check if en passant is available
    if(enPassantTargetRow == -1 || enPassantTargetCol == -1) return moves;
    
    bool isWhitePawn = isWhite(piece);
    
    // White pawns on 5th rank (row 3), black pawns on 4th rank (row 4)
    int correctRow = isWhitePawn ? 3 : 4;
    if(sRow != correctRow) return moves;
    
    // Check if pawn is adjacent to en passant target column
    if(abs(sCol - enPassantTargetCol) == 1) {
        // The target square is the en passant target
        moves.push_back(Move(sRow, sCol, enPassantTargetRow, enPassantTargetCol, EN_PASSANT));
    }
    
    return moves;
}

// Pawn promotion move generation
vector<Move> generatePawnPromotionMoves(int sRow, int sCol, int targetRow, int targetCol) {
    vector<Move> moves;
    
    // Check if pawn reaches promotion rank
    int piece = board[sRow][sCol];
    bool isWhitePawn = isWhite(piece);
    
    if((isWhitePawn && targetRow == 0) || (!isWhitePawn && targetRow == 7)) {
        // Generate all promotion options
        int queenPiece = isWhitePawn ? WHITE_QUEEN : BLACK_QUEEN;
        int rookPiece = isWhitePawn ? WHITE_ROOK : BLACK_ROOK;
        int bishopPiece = isWhitePawn ? WHITE_BISHOP : BLACK_BISHOP;
        int knightPiece = isWhitePawn ? WHITE_KNIGHT : BLACK_KNIGHT;
        
        moves.push_back(Move(sRow, sCol, targetRow, targetCol, PAWN_PROMOTION, queenPiece));
        moves.push_back(Move(sRow, sCol, targetRow, targetCol, PAWN_PROMOTION, rookPiece));
        moves.push_back(Move(sRow, sCol, targetRow, targetCol, PAWN_PROMOTION, bishopPiece));
        moves.push_back(Move(sRow, sCol, targetRow, targetCol, PAWN_PROMOTION, knightPiece));
    }
    
    return moves;
}

// Make a move on the board and update game state
void makeMove(const Move& move) {
    int movingPiece = board[move.startRow][move.startColumn];
    
    // Handle special moves
    switch(move.moveType) {
        case CASTLING_KINGSIDE:
            // Move king
            board[move.targetRow][move.targetColumn] = movingPiece;
            board[move.startRow][move.startColumn] = EMPTY;
            // Move rook
            board[move.targetRow][5] = board[move.targetRow][7];
            board[move.targetRow][7] = EMPTY;
            break;
            
        case CASTLING_QUEENSIDE:
            // Move king
            board[move.targetRow][move.targetColumn] = movingPiece;
            board[move.startRow][move.startColumn] = EMPTY;
            // Move rook
            board[move.targetRow][3] = board[move.targetRow][0];
            board[move.targetRow][0] = EMPTY;
            break;
            
        case EN_PASSANT:
            // Move pawn
            board[move.targetRow][move.targetColumn] = movingPiece;
            board[move.startRow][move.startColumn] = EMPTY;
            // Remove captured pawn (it's on the same rank as the moving pawn)
            board[move.startRow][move.targetColumn] = EMPTY;
            break;
            
        case PAWN_PROMOTION:
            // Replace pawn with promoted piece
            board[move.targetRow][move.targetColumn] = move.promotionPiece;
            board[move.startRow][move.startColumn] = EMPTY;
            break;
            
        default: // NORMAL move
            board[move.targetRow][move.targetColumn] = movingPiece;
            board[move.startRow][move.startColumn] = EMPTY;
            break;
    }
    
    // Update game state
    updateGameState(move);
}

// Update game state flags based on the move made
void updateGameState(const Move& move) {
    int movingPiece = board[move.targetRow][move.targetColumn];
    
    // Reset en passant target (will be set again if pawn moves two squares)
    enPassantTargetRow = -1;
    enPassantTargetCol = -1;
    
    // Track king and rook movement for castling rights
    if((movingPiece & 0b0111) == 0b0110) { // King moved
        if(isWhite(movingPiece)) {
            whiteKingMoved = true;
        } else {
            blackKingMoved = true;
        }
    }
    
    if((movingPiece & 0b0111) == 0b0010) { // Rook moved
        if(isWhite(movingPiece)) {
            if(move.startRow == 7 && move.startColumn == 0) whiteQueensideRookMoved = true;
            if(move.startRow == 7 && move.startColumn == 7) whiteKingsideRookMoved = true;
        } else {
            if(move.startRow == 0 && move.startColumn == 0) blackQueensideRookMoved = true;
            if(move.startRow == 0 && move.startColumn == 7) blackKingsideRookMoved = true;
        }
    }
    
    // Check for pawn double move (sets en passant target)
    if((movingPiece & 0b0111) == 0b0001) { // Pawn moved
        int moveDistance = abs(move.targetRow - move.startRow);
        if(moveDistance == 2) {
            // Pawn moved two squares, set en passant target
            enPassantTargetRow = (move.startRow + move.targetRow) / 2;
            enPassantTargetCol = move.startColumn;
        }
    }
}
