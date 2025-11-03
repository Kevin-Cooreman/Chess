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
)

# Headers (for IDE convenience)
set(HEADERS
    src/board.hpp
    src/moveGeneration.hpp
    src/game.hpp
    src/chessGUI.hpp
    src/evaluation.hpp
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

# Create console executable
add_executable(chess_console ${CONSOLE_SOURCES} ${HEADERS})

# Create GUI executable
add_executable(chess_gui ${GUI_SOURCES} ${HEADERS})

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
set_target_properties(chess_console chess_gui PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin"
)

# Print build info
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")