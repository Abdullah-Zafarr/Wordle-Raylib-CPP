# Project Background
This project was developed by my group and me as our 1st Semester Semester Project for the Programming Fundamentals (PF) course at NASTP NIIT Lahore. 

---

# Wordle++ Documentation   

A complete guide to understanding the Wordle++ codebase built with C++ and raylib.

---
I recommend using the **Markdown preview extension** to view this file. 
Once installed, press `Ctrl + Shift + V` to open the preview.

## 📺 [Watch the Gameplay Video with Audio](gameplay.mp4)

![Game Theme Preview](screenshots/menu.png)
![Game Theme Preview](loosinglayout/menu.png)

## Table of Contents

1. [Game Overview](#game-overview)
2. [What is Raylib?](#what-is-raylib)
3. [How to Compile and Run](#how-to-compile-and-run)
4. [Code Structure Overview](#code-structure-overview)
5. [Detailed Code Walkthrough](#detailed-code-walkthrough)
6. [Variable Reference](#variable-reference)
7. [Key Raylib Functions Used](#key-raylib-functions-used)

---

## Game Overview

Wordle++ is a clone of the popular word-guessing game "Wordle". The rules are simple:

1. You have **6 attempts** to guess a secret **5-letter word**
2. After each guess, the letters change color:
   - 🟩 **GREEN** = Correct letter in the correct position
   - 🟨 **YELLOW** = Correct letter but in the wrong position  
   - ⬛ **GRAY** = Letter is not in the word at all
3. Use the clues to figure out the word!

### Features
- Multiple color themes (Ocean, Sunset, Forest, Cyber, Midnight)
- Background music with 3 tracks (Chill, Epic, LoFi)
- Sound effects for typing, winning, and losing
- Leaderboard to track high scores
- Easy and Hard difficulty modes

---

## What is Raylib?

**Raylib** is a simple and easy-to-use library for making games in C/C++. It handles:

- **Window creation** - Creating the game window
- **Drawing** - Rendering shapes, text, and images
- **Input** - Detecting keyboard and mouse input
- **Audio** - Playing music and sound effects

Think of raylib as a toolbox that provides ready-made functions like `DrawRectangle()` or `IsKeyPressed()` so you don't have to worry about the low-level details.

---

## How to Compile and Run

### Prerequisites

1. **Install these tools** on your system:
   ```bash
   1.C++ Compiler: Install MinGW (64-bit) via MSYS2. Ensure g++ is added to your system PATH.

   2.CMake: Download and install from cmake.org.

   3.Raylib: Download the Windows installer from raylib.com. Important: Install it to C:\raylib as the project configuration is set to look for headers and libraries in C:/raylib/raylib.
   ```


### Compile the Game

```bash
Open your terminal (Command Prompt or PowerShell) in the project folder and run these commands:

PowerShell

# 1. Generate the build files
# Using -G "MinGW Makefiles" ensures CMake uses your MinGW compiler
cmake -S . -B build -G "MinGW Makefiles"

# 2. Compile the code
cmake --build build --clean-first
```

### Run the Game

```bash
# From the build directory
cd build
.\Wordle-Raylib.exe
```

---

## Code Structure Overview

The code in `main.cpp` is organized into **7 sections**:

| Section | Lines | Purpose |
|---------|-------|---------|
| 1. Configuration & Constants | ~20 | Game settings that never change (colors, grid size) |
| 2. Game Screens | ~10 | Enum defining all possible screens (Menu, Game, etc.) |
| 3. Data Structures | ~15 | Custom types for Tiles and Particles |
| 4. Global Variables | ~25 | Game state that changes during play |
| 5. Helper Functions | ~40 | Small utility functions (Txt, Btn, SpawnPart) |
| 6. File Handling | ~50 | Loading words and saving leaderboard |
| 7. Main Function | ~350 | Game loop, input handling, and rendering |

---

## Detailed Code Walkthrough

### Section 1: Configuration & Constants

```cpp
const int ROWS = 6;   // 6 guesses allowed
const int COLS = 5;   // 5 letters per word

const Color C_GRN = {106, 170, 100, 255};  // GREEN for correct
const Color C_YEL = {201, 180, 88, 255};   // YELLOW for wrong position
const Color C_GRY = {58, 58, 60, 255};     // GRAY for not in word
```

These are **constants** - values that never change while the game runs. Using constants makes it easy to tweak settings and keeps "magic numbers" out of the code.

**Colors in raylib** are defined as `{Red, Green, Blue, Alpha}` where each value is 0-255. Alpha is transparency (255 = fully visible).

---

### Section 2: Game Screens (Enum)

```cpp
enum Screen {
    MENU,   // Main menu
    NAME,   // Enter your name
    DIFF,   // Select difficulty
    MUSIC,  // Choose music
    THEME,  // Choose theme
    GAME,   // Play the game
    LEAD    // View leaderboard
};

Screen scr = MENU;  // Start at menu
```

An **enum** (enumeration) gives meaningful names to numbers. Instead of using `0`, `1`, `2` for screens, we use `MENU`, `NAME`, `DIFF` which are much easier to read.

---

### Section 3: Data Structures (Structs)

```cpp
struct Tile {
    char l = ' ';     // Letter displayed
    int s = 0;        // State: 0=empty, 1=yellow, 2=green, 3=gray
    bool rev = false; // Has the flip animation revealed the color?
    float t = 0;      // Time to start the flip animation
};
```

A **struct** groups related variables together. Each tile on the game board needs to track:
- What letter is displayed
- What color it should be after reveal
- Whether the flip animation has happened
- When to start the animation

---

### Section 4: Global Variables

These store the current state of the game:

```cpp
Tile grid[ROWS][COLS];    // The 6x5 game board
int curR = 0;             // Current row (which guess)
int curC = 0;             // Current column (which letter)
string ans;               // The secret word to guess
bool over = false;        // Is the game finished?
bool won = false;         // Did the player win?
```

---

### Section 5: Helper Functions

#### `Txt()` - Draw Centered Text
```cpp
void Txt(string t, float x, float y, int s, Color c)
```
Draws text centered at position (x, y) with size `s` and color `c`.

#### `Btn()` - Draw a Button
```cpp
bool Btn(string t, float x, float y, float w, float h, Color c)
```
Draws a button and returns `true` if clicked. The button:
- Has hover effects (lifts up slightly when mouse is over it)
- Changes brightness on hover
- Returns true only on the frame when clicked

#### `SpawnPart()` - Create Celebration Particles
```cpp
void SpawnPart(float x, float y)
```
Creates 15 particles that shoot out in random directions for the win celebration.

---

### Section 6: File Handling

#### `Load()` - Load Words from File
```cpp
vector<string> Load(string f)
```
Reads all 5-letter words from a text file and returns them in a list.

#### `Save()` - Save Score to Leaderboard
```cpp
void Save(int p)
```
Adds points to the player's score and updates the leaderboard file.

#### `Reset()` - Start New Game
```cpp
void Reset()
```
Picks a random word and clears the game board.

---

### Section 7: Main Function

The main function follows this structure:

```
1. INITIALIZATION
   ↓
2. GAME LOOP (repeats 60 times per second)
   ├── UPDATE (process input, update game state)
   └── DRAW (render everything on screen)
   ↓
3. CLEANUP (free resources)
```

#### The Game Loop

```cpp
while (!WindowShouldClose()) {
    // UPDATE
    if (aud) UpdateMusicStream(mus[musId]);
    // ... update particles, handle input
    
    // DRAW
    BeginDrawing();
    // ... draw background, tiles, keyboard
    EndDrawing();
}
```

This loop runs 60 times per second. Each iteration:
1. Processes any input (keyboard, mouse)
2. Updates game state (moves particles, etc.)
3. Draws everything fresh to the screen

---

### The Word Matching Algorithm

The most complex part of the game is checking guesses:

```cpp
// Step 1: Count letters in answer
int cnt[26] = {0};
for (char c : ans) cnt[c - 'A']++;

// Step 2: First pass - mark exact matches (GREEN)
for (int i = 0; i < 5; i++) {
    if (guess[i] == ans[i]) {
        tile.state = GREEN;
        cnt[guess[i] - 'A']--;  // Use up this letter
    }
}

// Step 3: Second pass - mark partial matches (YELLOW)
for (int i = 0; i < 5; i++) {
    if (tile.state != GREEN) {
        if (cnt[guess[i] - 'A'] > 0) {
            tile.state = YELLOW;
            cnt[guess[i] - 'A']--;
        } else {
            tile.state = GRAY;
        }
    }
}
```

**Why two passes?** Consider guessing "LLAMA" when the answer is "ALERT":
- First pass finds exact matches
- Second pass only marks YELLOW for letters that haven't been "used up"

---

## Variable Reference

| Variable | Type | Purpose |
|----------|------|---------|
| `grid[6][5]` | Tile[][] | The game board tiles |
| `curR` | int | Current row (0-5) |
| `curC` | int | Current column (0-4) |
| `ans` | string | The secret word |
| `pName` | string | Player's name |
| `score` | int | Total points earned |
| `lvl` | int | Current level number |
| `diff` | int | Difficulty (0=Easy, 1=Hard) |
| `theme` | int | Color theme (0-4) |
| `over` | bool | Is current game finished? |
| `won` | bool | Did player win? |
| `lStates[26]` | int[] | Keyboard letter colors (A-Z) |
| `scr` | Screen | Current screen (MENU, GAME, etc.) |

---

## Key Raylib Functions Used

### Window & System
| Function | What it does |
|----------|--------------|
| `InitWindow(w, h, title)` | Creates the game window |
| `CloseWindow()` | Closes the window |
| `SetTargetFPS(60)` | Limits game to 60 FPS |
| `WindowShouldClose()` | Returns true if user clicked X |

### Drawing
| Function | What it does |
|----------|--------------|
| `BeginDrawing()` | Start drawing a frame |
| `EndDrawing()` | Finish drawing and display |
| `DrawRectangle(x,y,w,h,color)` | Draw a filled rectangle |
| `DrawRectangleLinesEx(rect,thick,color)` | Draw rectangle outline |
| `DrawCircle(x,y,r,color)` | Draw a filled circle |
| `DrawTextEx(font,text,pos,size,spacing,color)` | Draw text with custom font |

### Input
| Function | What it does |
|----------|--------------|
| `IsKeyPressed(key)` | True if key was just pressed |
| `GetCharPressed()` | Get the character typed |
| `IsMouseButtonPressed(btn)` | True if mouse button just clicked |
| `GetMousePosition()` | Get current mouse coordinates |
| `GetMouseWheelMove()` | Get scroll wheel movement |

### Audio
| Function | What it does |
|----------|--------------|
| `InitAudioDevice()` | Initialize audio system |
| `LoadMusicStream(file)` | Load a music file |
| `PlayMusicStream(music)` | Start playing music |
| `UpdateMusicStream(music)` | Keep music playing (call each frame) |
| `LoadSound(file)` | Load a sound effect |
| `PlaySound(sound)` | Play a sound effect once |

### Utilities
| Function | What it does |
|----------|--------------|
| `GetTime()` | Seconds since game started |
| `GetRandomValue(min,max)` | Random number in range |
| `CheckCollisionPointRec(point,rect)` | Is point inside rectangle? |
| `ColorBrightness(color,amount)` | Make color lighter/darker |
| `ColorAlpha(color,alpha)` | Set color transparency |
| `Fade(color,alpha)` | Fade color to transparent |

---

## Tips for Learning

1. **Run the code** - Play the game first to understand what it does
2. **Add print statements** - Use `printf()` to see values while running
3. **Change constants** - Try changing `ROWS` to 4 or colors to see effects
4. **Comment out sections** - See what breaks to understand dependencies
5. **Read raylib docs** - Visit [raylib.com](https://www.raylib.com/cheatsheet/cheatsheet.html) for the cheatsheet

---

Happy coding! 🎮
