#include "moveGeneration.hpp"
#include <iostream>

using namespace std;

vector<Move> generateRookMoves(int sRow, int sCol){
    vector<Move> moves;

    //up
    for(int targetRow = sRow -1; targetRow >= 0; targetRow--){

        if(sameColour(board[sRow][sCol], board[targetRow][sCol])){
            break;
        }

        else if(!sameColour(board[sRow][sCol], board[targetRow][sCol]) && !isEmpty(board[targetRow][sCol])){
            moves.push_back(Move{sRow, sCol, targetRow, sCol});
            break;
        }

        else{
            moves.push_back(Move{sRow, sCol, targetRow, sCol});
        }
    }
    //down
    for(int targetRow = sRow +1; targetRow < 8; targetRow++){

        if(sameColour(board[sRow][sCol], board[targetRow][sCol])){
            break;
        }

        else if(!sameColour(board[sRow][sCol], board[targetRow][sCol]) && !isEmpty(board[targetRow][sCol])){
            moves.push_back(Move{sRow, sCol, targetRow, sCol});
            break;
        }

        else{
            moves.push_back(Move{sRow, sCol, targetRow, sCol});
        }
    }

    //left
    for(int targetCol = sCol -1; targetCol >= 0; targetCol--){

        if(sameColour(board[sRow][sCol], board[sRow][targetCol])){
            break;
        }

        else if(!sameColour(board[sRow][sCol], board[sRow][targetCol]) && !isEmpty(board[sRow][targetCol])){
            moves.push_back(Move{sRow, sCol, sRow, targetCol});
            break;
        }

        else{
            moves.push_back(Move{sRow, sCol, sRow, targetCol});
        }
    }

    //right
    for(int targetCol = sCol +1; targetCol < 8; targetCol++){

        if(sameColour(board[sRow][sCol], board[sRow][targetCol])){
            break;
        }

        else if(!sameColour(board[sRow][sCol], board[sRow][targetCol]) && !isEmpty(board[sRow][targetCol])){
            moves.push_back(Move{sRow, sCol, sRow, targetCol});
            break;
        }

        else{
            moves.push_back(Move{sRow, sCol, sRow, targetCol});
        }
    }
    return moves;
}

vector<Move> generateBishopMoves(int sRow, int sCol){
    vector<Move> moves;

    for(int i = 1; i < 8; i++){
        int targetRow = sRow - i;  // up
        int targetCol = sCol - i;  // left
        
        if(targetRow < 0 || targetCol < 0) break; // Off board
        
        if(sameColour(board[sRow][sCol], board[targetRow][targetCol])){
            break;
        }
        else if(!sameColour(board[sRow][sCol], board[targetRow][targetCol]) && !isEmpty(board[targetRow][targetCol])){
            moves.push_back(Move{sRow, sCol, targetRow, targetCol});
            break;
        }
        else{
            moves.push_back(Move{sRow, sCol, targetRow, targetCol});
        }
    }

    for(int i = 1; i < 8; i++){
        int targetRow = sRow - i;  // up
        int targetCol = sCol + i;  // right
        
        if(targetRow < 0 || targetCol > 7) break; // Off board
        
        if(sameColour(board[sRow][sCol], board[targetRow][targetCol])){
            break;
        }
        else if(!sameColour(board[sRow][sCol], board[targetRow][targetCol]) && !isEmpty(board[targetRow][targetCol])){
            moves.push_back(Move{sRow, sCol, targetRow, targetCol});
            break;
        }
        else{
            moves.push_back(Move{sRow, sCol, targetRow, targetCol});
        }
    }

    for(int i = 1; i < 8; i++){
        int targetRow = sRow + i;  // down
        int targetCol = sCol + i;  // right
        
        if(targetRow > 7 || targetCol > 7) break; // Off board
        
        if(sameColour(board[sRow][sCol], board[targetRow][targetCol])){
            break;
        }
        else if(!sameColour(board[sRow][sCol], board[targetRow][targetCol]) && !isEmpty(board[targetRow][targetCol])){
            moves.push_back(Move{sRow, sCol, targetRow, targetCol});
            break;
        }
        else{
            moves.push_back(Move{sRow, sCol, targetRow, targetCol});
        }
    }

    for(int i = 1; i < 8; i++){
        int targetRow = sRow + i;  // down
        int targetCol = sCol - i;  // left
        
        if(targetRow > 7 || targetCol < 0) break; // Off board
        
        if(sameColour(board[sRow][sCol], board[targetRow][targetCol])){
            break;
        }
        else if(!sameColour(board[sRow][sCol], board[targetRow][targetCol]) && !isEmpty(board[targetRow][targetCol])){
            moves.push_back(Move{sRow, sCol, targetRow, targetCol});
            break;
        }
        else{
            moves.push_back(Move{sRow, sCol, targetRow, targetCol});
        }
    }

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
                moves.push_back(Move{sRow, sCol, targetRow, targetCol});
            }
        }
    }
    
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
                moves.push_back(Move{sRow, sCol, targetRow, targetCol});
            }
        }
    }
    
    return moves;
}

vector<Move> generatePawnMoves(int sRow, int sCol){
    //To Do: add pawn move generation
}
