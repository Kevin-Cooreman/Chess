#include <iostream>
#include "game.hpp"
#include "chessGUI.hpp"

int main() {
    std::cout << "Starting Chess Engine with SFML GUI...\n";
    
    try {
        ChessGame game;
        ChessGUI gui(game);
        
        std::cout << "GUI initialized successfully!\n";
        std::cout << "Controls:\n";
        std::cout << "- Click to select pieces and make moves\n";
        std::cout << "- ESC to deselect current piece\n";
        std::cout << "- Close window to exit\n\n";
        
        gui.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Thanks for playing!\n";
    return 0;
}