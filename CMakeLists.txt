cmake_minimum_required(VERSION 3.11)
project(Wordle-Raylib VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# --- 1. Point to your Local Raylib ---
# The installer usually puts headers and libs in this folder
set(RAYLIB_PATH "C:/raylib/raylib")

# Tell CMake where to find 'raylib.h'
include_directories(${RAYLIB_PATH}/src)

# Tell Linker where to find 'libraylib.a'
link_directories(${RAYLIB_PATH}/src)

# --- 2. Setup Executable ---
add_executable(${PROJECT_NAME} main.cpp)

# --- 3. Link Libraries ---
# Raylib on Windows needs these system libraries to run
target_link_libraries(${PROJECT_NAME} PRIVATE 
    raylib 
    opengl32 
    gdi32 
    winmm
)

# --- 4. Assets Copy (Same as before) ---
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_BINARY_DIR}/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/fonts
        ${CMAKE_BINARY_DIR}/fonts
)