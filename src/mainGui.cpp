#include <iostream>
#include "game.hpp"
#include "chessGUI.hpp"

int main() {
    std::cout << "Starting Chess Engine with SFML GUI...\n\n";
    
    std::cout << "=== CHESS ENGINE ===\n";
    std::cout << "Select game mode:\n";
    std::cout << "  1 - Player vs Player\n";
    std::cout << "  2 - Player (White) vs Engine (Black)\n";
    std::cout << "  3 - Engine (White) vs Player (Black)\n";
    std::cout << "  4 - Engine vs Engine\n";
    std::cout << "Enter choice (1-4): ";
    
    int mode = 1;
    std::string input;
    std::getline(std::cin, input);
    if (!input.empty()) {
        mode = std::stoi(input);
        if (mode < 1 || mode > 4) mode = 1;
    }
    
    int engineDepth = 3;
    if (mode != 1) {
        std::cout << "Enter engine search depth (1-5, default 3): ";
        std::getline(std::cin, input);
        if (!input.empty()) {
            engineDepth = std::stoi(input);
            if (engineDepth < 1) engineDepth = 1;
            if (engineDepth > 5) engineDepth = 5;
        }
    }
    
    try {
        ChessGame game;
        ChessGUI gui(game);
        
        // Set engine mode based on selection
        bool enginePlaysWhite = (mode == 3 || mode == 4);
        bool enginePlaysBlack = (mode == 2 || mode == 4);
        gui.setEngineMode(enginePlaysWhite, enginePlaysBlack, engineDepth);
        
        std::cout << "\nGUI initialized successfully!\n";
        std::cout << "Controls:\n";
        std::cout << "- Click to select pieces and make moves\n";
        std::cout << "- ESC to deselect current piece\n";
        std::cout << "- Close window to exit\n\n";
        
        if (mode == 2) {
            std::cout << "You are playing as White against the engine.\n";
        } else if (mode == 3) {
            std::cout << "Engine is playing as White. You are Black.\n";
        } else if (mode == 4) {
            std::cout << "Watching: Engine vs Engine\n";
        }
        std::cout << std::endl;
        
        gui.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Thanks for playing!\n";
    return 0;
}