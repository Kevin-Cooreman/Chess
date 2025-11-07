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

# SFML Configuration
set(SFML_ROOT "${CMAKE_SOURCE_DIR}/external/SFML-2.6.1")
set(SFML_INCLUDE_DIR "${SFML_ROOT}/include")
set(SFML_LIBRARY_DIR "${SFML_ROOT}/lib")

# Find SFML libraries (debug versions for Debug build)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(SFML_GRAPHICS_LIB "${SFML_LIBRARY_DIR}/libsfml-graphics-d.a")
    set(SFML_WINDOW_LIB "${SFML_LIBRARY_DIR}/libsfml-window-d.a")
    set(SFML_SYSTEM_LIB "${SFML_LIBRARY_DIR}/libsfml-system-d.a")
else()
    set(SFML_GRAPHICS_LIB "${SFML_LIBRARY_DIR}/libsfml-graphics.a")
    set(SFML_WINDOW_LIB "${SFML_LIBRARY_DIR}/libsfml-window.a")
    set(SFML_SYSTEM_LIB "${SFML_LIBRARY_DIR}/libsfml-system.a")
endif()

# Core chess engine sources (shared between console and GUI)
set(CORE_SOURCES
    src/board.cpp
    src/moveGeneration.cpp
    src/game.cpp
    src/evaluation.cpp
    src/engine.cpp
    src/engine_v1.cpp
)

# Headers (for IDE convenience)
set(HEADERS
    src/board.hpp
    src/moveGeneration.hpp
    src/game.hpp
    src/chessGUI.hpp
    src/evaluation.hpp
    src/engine.hpp
    src/engine_v1.hpp
)

# Console version
set(CONSOLE_SOURCES
    src/main.cpp
    ${CORE_SOURCES}
)

# GUI version sources
set(GUI_SOURCES
    src/mainGUI.cpp
    src/chessGUI.cpp
    ${CORE_SOURCES}
)

# Tuning version sources
set(TUNING_SOURCES
    src/tuning.cpp
    ${CORE_SOURCES}
)

# Benchmark version sources
set(BENCHMARK_SOURCES
    src/benchmark.cpp
    ${CORE_SOURCES}
)

# Genetic tuning version sources
set(GENETIC_SOURCES
    src/genetic_tuning.cpp
    ${CORE_SOURCES}
)

# Genetic PST Evolution sources
set(GENETIC_PST_SOURCES
    src/genetic_pst.cpp
    ${CORE_SOURCES}
)

# Version comparison sources
set(COMPARE_SOURCES
    src/compare_versions.cpp
    src/evaluation_v1.cpp
    ${CORE_SOURCES}
)

# Speed comparison sources (Zobrist vs FEN)
set(SPEED_SOURCES
    src/speed_comparison.cpp
    src/evaluation_v1.cpp
    ${CORE_SOURCES}
)

# Test Zobrist hash stability
set(TEST_ZOBRIST_SOURCES
    src/test_zobrist.cpp
    ${CORE_SOURCES}
)

# Test TT functionality
set(TEST_TT_SOURCES
    src/test_tt.cpp
    ${CORE_SOURCES}
)

# Test evaluation sanity
set(TEST_EVAL_SOURCES
    src/test_eval.cpp
    ${CORE_SOURCES}
)

# Test board state
set(TEST_BOARD_SOURCES
    src/test_board.cpp
    ${CORE_SOURCES}
)

# Test queen capture
set(TEST_QUEEN_SOURCES
    src/test_queen.cpp
    ${CORE_SOURCES}
)

# Test hash during search
set(TEST_HASH_SEARCH_SOURCES
    src/test_hash_search.cpp
    ${CORE_SOURCES}
)

# Test full evaluation
set(TEST_FULL_EVAL_SOURCES
    src/test_full_eval.cpp
    ${CORE_SOURCES}
)

# Test engine self-play
set(TEST_SELFPLAY_SOURCES
    src/test_selfplay.cpp
    ${CORE_SOURCES}
)

# Test tactical ability
set(TEST_TACTICS_SOURCES
    src/test_tactics.cpp
    ${CORE_SOURCES}
)

set(TEST_SIMPLE_CAPTURE_SOURCES
    src/test_simple_capture.cpp
    ${CORE_SOURCES}
)

# Create console executable
add_executable(chess_console ${CONSOLE_SOURCES} ${HEADERS})

# Create GUI executable
add_executable(chess_gui ${GUI_SOURCES} ${HEADERS})

# Create tuning executable
add_executable(chess_tuning ${TUNING_SOURCES} ${HEADERS})

# Create benchmark executable
add_executable(chess_benchmark ${BENCHMARK_SOURCES} ${HEADERS})

# Create genetic tuning executable
add_executable(chess_genetic ${GENETIC_SOURCES} ${HEADERS})

# Create genetic PST evolution executable
add_executable(chess_genetic_pst ${GENETIC_PST_SOURCES} ${HEADERS})

# Create version comparison executable
add_executable(chess_compare ${COMPARE_SOURCES} ${HEADERS})

# Create speed comparison executable (Zobrist vs FEN)
add_executable(chess_speed ${SPEED_SOURCES} ${HEADERS})

# Create Zobrist test executable
add_executable(test_zobrist ${TEST_ZOBRIST_SOURCES} ${HEADERS})

# Create TT test executable
add_executable(test_tt ${TEST_TT_SOURCES} ${HEADERS})

# Create evaluation test executable  
add_executable(test_eval ${TEST_EVAL_SOURCES} ${HEADERS})

# Create board state test executable
add_executable(test_board ${TEST_BOARD_SOURCES} ${HEADERS})

# Create queen capture test executable
add_executable(test_queen ${TEST_QUEEN_SOURCES} ${HEADERS})

# Create hash search test executable
add_executable(test_hash_search ${TEST_HASH_SEARCH_SOURCES} ${HEADERS})

# Create full evaluation test executable
add_executable(test_full_eval ${TEST_FULL_EVAL_SOURCES} ${HEADERS})

# Create self-play test executable
add_executable(test_selfplay ${TEST_SELFPLAY_SOURCES} ${HEADERS})

# Create tactical ability test executable
add_executable(test_tactics ${TEST_TACTICS_SOURCES} ${HEADERS})

# Create simple capture test executable
add_executable(test_simple_capture ${TEST_SIMPLE_CAPTURE_SOURCES} ${HEADERS})

# Include SFML headers for GUI version
target_include_directories(chess_gui PRIVATE ${SFML_INCLUDE_DIR})

# Link SFML libraries for GUI version
target_link_libraries(chess_gui 
    ${SFML_GRAPHICS_LIB}
    ${SFML_WINDOW_LIB}
    ${SFML_SYSTEM_LIB}
)

# Copy SFML DLLs to output directory
set(SFML_DLL_DIR "${SFML_ROOT}/bin")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(SFML_DLLS 
        "${SFML_DLL_DIR}/sfml-graphics-d-2.dll"
        "${SFML_DLL_DIR}/sfml-window-d-2.dll" 
        "${SFML_DLL_DIR}/sfml-system-d-2.dll")
else()
    set(SFML_DLLS 
        "${SFML_DLL_DIR}/sfml-graphics-2.dll"
        "${SFML_DLL_DIR}/sfml-window-2.dll"
        "${SFML_DLL_DIR}/sfml-system-2.dll")
endif()

# Custom command to copy DLLs
foreach(DLL ${SFML_DLLS})
    add_custom_command(TARGET chess_gui POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${DLL} $<TARGET_FILE_DIR:chess_gui>)
endforeach()

# Copy assets directory to build directory
add_custom_command(TARGET chess_gui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:chess_gui>/assets)

# Set output directories
set_target_properties(chess_console chess_gui chess_tuning chess_benchmark chess_genetic chess_genetic_pst chess_compare chess_speed test_zobrist test_tt test_eval test_board test_queen test_hash_search test_full_eval test_selfplay test_tactics test_simple_capture PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin"
)

# Print build info
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")