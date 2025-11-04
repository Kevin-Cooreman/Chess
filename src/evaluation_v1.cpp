// evaluation_v1.cpp - Baseline version for benchmarking
// This is the original evaluation with PST optimization
// Material=10, Position=10, KingSafety=5, PawnStructure=0.1

#include "evaluation_v1.hpp"
#include "game.hpp"
#include <cctype>

using namespace std;

// Piece-square tables
static const int pawnPST[8][8] = {
    {  0,  0,  0,  0,  0,  0,  0,  0},
    { 50, 50, 50, 50, 50, 50, 50, 50},
    { 10, 10, 20, 30, 30, 20, 10, 10},
    {  5,  5, 10, 25, 25, 10,  5,  5},
    {  0,  0,  0, 20, 20,  0,  0,  0},
    {  5, -5,-10,  0,  0,-10, -5,  5},
    {  5, 10, 10,-20,-20, 10, 10,  5},
    {  0,  0,  0,  0,  0,  0,  0,  0}
};

static const int knightPST[8][8] = {
    {-50,-40,-30,-30,-30,-30,-40,-50},
    {-40,-20,  0,  0,  0,  0,-20,-40},
    {-30,  0, 10, 15, 15, 10,  0,-30},
    {-30,  5, 15, 20, 20, 15,  5,-30},
    {-30,  0, 15, 20, 20, 15,  0,-30},
    {-30,  5, 10, 15, 15, 10,  5,-30},
    {-40,-20,  0,  5,  5,  0,-20,-40},
    {-50,-40,-30,-30,-30,-30,-40,-50}
};

static const int bishopPST[8][8] = {
    {-20,-10,-10,-10,-10,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5, 10, 10,  5,  0,-10},
    {-10,  5,  5, 10, 10,  5,  5,-10},
    {-10,  0, 10, 10, 10, 10,  0,-10},
    {-10, 10, 10, 10, 10, 10, 10,-10},
    {-10,  5,  0,  0,  0,  0,  5,-10},
    {-20,-10,-10,-10,-10,-10,-10,-20}
};

static const int rookPST[8][8] = {
    {  0,  0,  0,  0,  0,  0,  0,  0},
    {  5, 10, 10, 10, 10, 10, 10,  5},
    { -5,  0,  0,  0,  0,  0,  0, -5},
    { -5,  0,  0,  0,  0,  0,  0, -5},
    { -5,  0,  0,  0,  0,  0,  0, -5},
    { -5,  0,  0,  0,  0,  0,  0, -5},
    { -5,  0,  0,  0,  0,  0,  0, -5},
    {  0,  0,  0,  5,  5,  0,  0,  0}
};

static const int queenPST[8][8] = {
    {-20,-10,-10, -5, -5,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5,  5,  5,  5,  0,-10},
    { -5,  0,  5,  5,  5,  5,  0, -5},
    {  0,  0,  5,  5,  5,  5,  0, -5},
    {-10,  5,  5,  5,  5,  5,  0,-10},
    {-10,  0,  5,  0,  0,  0,  0,-10},
    {-20,-10,-10, -5, -5,-10,-10,-20}
};

static const int kingPST[8][8] = {
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-20,-30,-30,-40,-40,-30,-30,-20},
    {-10,-20,-20,-20,-20,-20,-20,-10},
    { 20, 20,  0,  0,  0,  0, 20, 20},
    { 20, 30, 10,  0,  0, 10, 30, 20}
};

double EvaluationV1::materialCount(const ChessGame& game) const {
    string fen = game.getCurrentFEN();
    size_t spacePos = fen.find(' ');
    string piecePlacement = fen.substr(0, spacePos);

    double evaluation = 0.0;

    for(char c : piecePlacement) {
        if(c == '/') continue;
        if(isdigit(c)) continue;

        double value = 0.0;
        char piece = tolower(c);

        switch(piece) {
            case 'p': value = PAWN_VALUE; break;
            case 'n': value = KNIGHT_VALUE; break;
            case 'b': value = BISHOP_VALUE; break;
            case 'r': value = ROOK_VALUE; break;
            case 'q': value = QUEEN_VALUE; break;
            case 'k': value = 0; break;
            default: continue;
        }

        if(isupper(c)) {
            evaluation += value;
        } else {
            evaluation -= value;
        }
    }

    return evaluation;
}

double EvaluationV1::position(const ChessGame& game) const {
    double positionValue = 0.0;
    string fen = game.getCurrentFEN();
    size_t spacePos = fen.find(' ');
    string piecePlacement = fen.substr(0, spacePos);

    int row = 0, col = 0;

    for(char c : piecePlacement) {
        if(c == '/') {
            row++;
            col = 0;
            continue;
        }
        else if(isdigit(c)) {
            col += (c - '0');
            continue;
        }
        
        double pieceValue = 0;
        char piece = tolower(c);
        bool isWhite = isupper(c);
        int pstRow = isWhite ? row : (7 - row);
        
        switch(piece) {
            case 'p': pieceValue = pawnPST[pstRow][col] / 100.0; break;
            case 'n': pieceValue = knightPST[pstRow][col] / 100.0; break;
            case 'b': pieceValue = bishopPST[pstRow][col] / 100.0; break;
            case 'r': pieceValue = rookPST[pstRow][col] / 100.0; break;
            case 'q': pieceValue = queenPST[pstRow][col] / 100.0; break;
            case 'k': pieceValue = kingPST[pstRow][col] / 100.0; break;
            default: 
                col++;
                continue;
        }
        
        if(isWhite) {
            positionValue += pieceValue;
        } else {
            positionValue -= pieceValue;
        }
        
        col++;
    }

    return positionValue;
}

double EvaluationV1::kingsafety(const ChessGame& game) const {
    string fen = game.getCurrentFEN();
    double kingSafety = 0.0;

    size_t wKingPos = fen.find('K');
    if (wKingPos != string::npos) {
        if (wKingPos < 8 || (wKingPos >= 7*9 && wKingPos < 8*9)) {
            kingSafety += 0.5;
        }
    }

    size_t bKingPos = fen.find('k');
    if (bKingPos != string::npos) {
        if (bKingPos < 8 || (bKingPos >= 7*9 && bKingPos < 8*9)) {
            kingSafety -= 0.5;
        }
    }

    size_t castlingStart = fen.find(' ') + 1;
    size_t castlingEnd = fen.find(' ', castlingStart);
    string castling = fen.substr(castlingStart, castlingEnd - castlingStart);

    if (castling.find('K') == string::npos && castling.find('Q') == string::npos) {
        kingSafety += 0.3;
    }
    if (castling.find('k') == string::npos && castling.find('q') == string::npos) {
        kingSafety -= 0.3;
    }

    return kingSafety;
}

double EvaluationV1::pawnStructure(const ChessGame& game) const {
    string fen = game.getCurrentFEN();
    size_t spacePos = fen.find(' ');
    string piecePlacement = fen.substr(0, spacePos);

    int whitePawnsPerFile[8] = {0};
    int blackPawnsPerFile[8] = {0};

    int row = 0, col = 0;
    for(char c : piecePlacement) {
        if(c == '/') {
            row++;
            col = 0;
            continue;
        }
        if(isdigit(c)) {
            col += (c - '0');
            continue;
        }

        if(c == 'P' && col < 8) whitePawnsPerFile[col]++;
        if(c == 'p' && col < 8) blackPawnsPerFile[col]++;

        col++;
    }

    double structureScore = 0.0;

    for(int i = 0; i < 8; i++) {
        if(whitePawnsPerFile[i] > 1) structureScore -= (whitePawnsPerFile[i] - 1);
        if(blackPawnsPerFile[i] > 1) structureScore += (blackPawnsPerFile[i] - 1);
    }

    return structureScore;
}

double EvaluationV1::evaluate(const ChessGame& game) const {
    double evaluation = 0.0;
    evaluation += 10.0 * materialCount(game);
    evaluation += 10.0 * position(game);
    evaluation += 5.0 * kingsafety(game);
    evaluation += 0.1 * pawnStructure(game);
    return evaluation;
}
