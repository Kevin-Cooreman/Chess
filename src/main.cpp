#include <iostream>
#include <string>
#include "board.hpp"
#include "game.hpp"
#include "engine.hpp"

using namespace std;

void printInstructions() {
    cout << "\n=== CHESS ENGINE ===\n";
    cout << "Commands:\n";
    cout << "  - Move: e2e4, e2-e4, etc. (from square to square)\n";
    cout << "  - Promotion: e7e8q (optional) or interactive choice when pawn reaches end\n";
    cout << "  - Castling: e1g1 (king moves two squares)\n";
    cout << "  - 'moves' - Show all legal moves\n";
    cout << "  - 'fen' - Show current FEN string\n";
    cout << "  - 'quit' - Exit the game\n";
    cout << "  - 'help' - Show this help\n";
    cout << "====================\n";
}

int main() {
    ChessGame game;
    string input;
    
    cout << "\n=== CHESS ENGINE ===\n";
    cout << "Select game mode:\n";
    cout << "  1 - Player vs Player\n";
    cout << "  2 - Player (White) vs Engine (Black)\n";
    cout << "  3 - Engine (White) vs Player (Black)\n";
    cout << "Enter choice: ";
    
    int mode = 1;
    getline(cin, input);
    if (!input.empty()) {
        mode = stoi(input);
    }
    
    bool enginePlaysWhite = (mode == 3);
    bool enginePlaysBlack = (mode == 2 || mode == 3);
    int engineDepth = 3; // Search depth for the engine
    
    Engine engine;
    
    printInstructions();
    
    while (!game.isGameOver()) {
        game.displayBoard();
        
        // Check if it's the engine's turn
        bool isEngineTurn = (game.isWhiteToMove() && enginePlaysWhite) || 
                           (!game.isWhiteToMove() && enginePlaysBlack);
        
        if (isEngineTurn) {
            cout << "\nEngine is thinking";
            cout.flush();
            for (int i = 0; i < 3; i++) {
                cout << ".";
                cout.flush();
            }
            cout << endl;
            
            Move bestMove = engine.getBestMove(game, engineDepth);
            
            if (bestMove.startRow == -1) {
                cout << "Engine has no legal moves!\n";
                break;
            }
            
            game.makeEngineMove(bestMove);
            continue;
        }
        
        // Human player's turn
        cout << "Enter move (or 'help' for commands): ";
        
        // Check if input stream is still valid
        if (!getline(cin, input)) {
            cout << "\nInput stream ended. Exiting game.\n";
            break;
        }
        
        if (input == "quit" || input == "exit") {
            cout << "Thanks for playing!\n";
            break;
        } else if (input == "help") {
            printInstructions();
        } else if (input == "moves") {
            game.displayLegalMoves();
        } else if (input == "fen") {
            cout << "FEN: " << game.getCurrentFEN() << "\n";
        } else if (input.empty()) {
            // If input is empty but stream is still valid, just continue
            cout << "Please enter a move or command.\n";
            continue;
        } else {
            if (!game.makePlayerMove(input)) {
                cout << "Try 'moves' to see legal moves or 'help' for commands.\n";
            }
        }
    }
    
    if (game.isGameOver()) {
        game.displayBoard();
        cout << "\n*** GAME OVER ***\n";
        cout << game.getGameResult() << endl;
    }
    
    return 0;
}