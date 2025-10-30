#include <iostream>
#include <string>
#include "board.hpp"
#include "game.hpp"

using namespace std;

int main() {
    ChessGame game;
    string input;
    int loopCount = 0;
    
    cout << "Starting game loop test...\n";
    
    while (!game.isGameOver() && loopCount < 5) {
        loopCount++;
        cout << "Loop iteration: " << loopCount << "\n";
        
        cout << "About to display board...\n";
        game.displayBoard();
        cout << "Board displayed.\n";
        
        cout << "Prompting for input...\n";
        cout << "Enter move: ";
        
        // Simulate user input - let's try e2e4
        if (loopCount == 1) {
            input = "e2e4";
            cout << input << "\n";
        } else if (loopCount == 2) {
            input = "e7e5";
            cout << input << "\n";
        } else {
            input = "quit";
            cout << input << "\n";
        }
        
        cout << "Processing input: '" << input << "'\n";
        
        if (input == "quit") {
            cout << "Quitting...\n";
            break;
        } else {
            cout << "Attempting to make move...\n";
            bool success = game.makePlayerMove(input);
            cout << "Move result: " << (success ? "success" : "failed") << "\n";
        }
    }
    
    cout << "Loop ended. Game over: " << game.isGameOver() << "\n";
    return 0;
}