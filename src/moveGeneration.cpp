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
