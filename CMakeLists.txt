cmake_minimum_required(VERSION 3.20)
project(ChessEngine VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Source files
set(SOURCES
    src/main.cpp
    src/board.cpp
    src/moveGeneration.cpp
    src/game.cpp
)

# Headers (for IDE convenience)
set(HEADERS
    src/board.hpp
    src/moveGeneration.hpp
    src/game.hpp
)

# Common library sources (shared by all executables)
set(LIB_SOURCES
    src/board.cpp
    src/moveGeneration.cpp
    src/game.cpp
)

# Create main executable
add_executable(chess_engine ${SOURCES} ${HEADERS})

# Create test and demo executables
add_executable(chess_test src/test.cpp ${LIB_SOURCES})
add_executable(chess_debug src/debug.cpp ${LIB_SOURCES})
add_executable(chess_demo_promotion src/demo_promotion.cpp ${LIB_SOURCES})
add_executable(chess_test_promotion src/test_promotion.cpp ${LIB_SOURCES})
add_executable(chess_loop_test src/loop_test.cpp ${LIB_SOURCES})

# Set output directories for all executables
set_target_properties(chess_engine chess_test chess_debug chess_demo_promotion chess_test_promotion chess_loop_test
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin"
)

# Print build info
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")