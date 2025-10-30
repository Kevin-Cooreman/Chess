#include <iostream>
#include <string>
#include "board.hpp"
#include "game.hpp"

using namespace std;

void printInstructions() {
    cout << "\n=== CHESS ENGINE ===\n";
    cout << "Commands:\n";
    cout << "  - Move: e2e4, e2-e4, etc. (from square to square)\n";
    cout << "  - Promotion: e7e8q (optional) or interactive choice when pawn reaches end\n";
    cout << "  - Castling: e1g1 (king moves two squares)\n";
    cout << "  - 'moves' - Show all legal moves\n";
    cout << "  - 'quit' - Exit the game\n";
    cout << "  - 'help' - Show this help\n";
    cout << "====================\n";
}

int main() {
    ChessGame game;
    string input;
    
    printInstructions();
    
    while (!game.isGameOver()) {
        game.displayBoard();
        
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