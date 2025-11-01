# Chess Engine

A complete chess engine built from scratch in C++17 with a console-based interface.

## Features

- ♟️ Complete chess rule implementation
- 🏰 All special moves (castling, en passant, pawn promotion)
- ✅ Legal move validation and check/checkmate detection
- 🖥️ Enhanced console interface with Unicode chess board display
- 🎮 Interactive gameplay with intuitive move input

## Project Structure

```
src/
├── main.cpp           # Console-based chess game interface
├── board.hpp/cpp      # Chess board representation and game state
├── moveGeneration.hpp/cpp  # Move generation and validation
└── game.hpp/cpp       # High-level game management
```

## Building and Running

### Prerequisites
- C++17 compatible compiler (GCC/MinGW recommended)
- CMake 3.20 or higher

### Build Instructions

1. Clone the repository:
   ```bash
   git clone <your-repo-url>
   cd Chess
   ```

2. Configure and build:
   ```bash
   cmake -B build -G "MinGW Makefiles"
   cmake --build build
   ```

3. Run the chess engine:
   ```bash
   ./build/bin/chess_engine.exe
   ```

## How to Play

### Move Input Formats
- **Standard moves**: `e2e4`, `e2-e4`, `Nf3`, etc.
- **Castling**: `e1g1` (king side), `e1c1` (queen side)
- **Pawn promotion**: `e7e8q` (specify piece) or interactive choice
- **En passant**: Handled automatically when legal

### Available Commands
- `moves` - Show all legal moves for current player
- `help` - Display command help
- `quit` - Exit the game

### Game Features
- Automatic check detection and display
- Checkmate and stalemate recognition
- Interactive pawn promotion with piece selection
- Full legal move validation
- Turn-based gameplay with clear board display

## Technical Details

- **Language**: C++17
- **Build System**: CMake
- **Architecture**: Modular design with separate board representation, move generation, and game logic
- **Platform**: Cross-platform console application

## Board Representation

The engine uses an efficient board representation with:
- Binary piece encoding for fast operations
- Special move tracking (castling rights, en passant)
- Complete game state management
- Unicode symbols for clear visual display

## Development

The project is structured for easy extension and modification:
- Clean separation between engine logic and interface
- Comprehensive move validation system
- Extensible game state management
- Well-documented code with clear APIs


