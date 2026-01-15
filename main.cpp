// ============================================================================
// WORDLE++ - A Wordle Clone Built with Raylib
// ============================================================================
// This game is a recreation of the popular word game "Wordle" where players
// try to guess a 5-letter word in 6 attempts. After each guess, tiles change
// color to show how close you are to the answer:
//   - GREEN  = Correct letter in the correct position
//   - YELLOW = Correct letter but in the wrong position
//   - GRAY   = Letter is not in the word at all
// ============================================================================

#include <bits/stdc++.h>  // Includes all standard library headers (string, vector, fstream, etc.)
#include "raylib.h"       // The raylib library for graphics, audio, and input
using namespace std;      // Allows us to use 'string' instead of 'std::string', etc.


// ============================================================================
// SECTION 1: GAME CONFIGURATION & CONSTANTS
// ============================================================================
// These values control the basic game settings and never change during gameplay

const int ROWS = 6;   // Number of guesses allowed (6 rows on the game board)
const int COLS = 5;   // Number of letters per word (5 columns for 5-letter words)

// Tile state colors (used to show feedback after each guess)
const Color C_GRN = {106, 170, 100, 255};  // GREEN - correct letter, correct position
const Color C_YEL = {201, 180, 88, 255};   // YELLOW - correct letter, wrong position
const Color C_GRY = {58, 58, 60, 255};     // GRAY - letter not in word
const Color C_ACC = {139, 92, 246, 255};   // ACCENT color for UI buttons (purple)

// Theme colors - Each theme has 3 gradient colors [top, middle, bottom]
// Used to create the background gradient effect
const Color THEMES[5][3] = {
    // Theme 0: Ocean (cyan/blue tones)
    {{6, 182, 212, 255}, {14, 116, 144, 255}, {8, 47, 73, 255}},
    // Theme 1: Sunset (orange/pink/purple)
    {{251, 146, 60, 255}, {236, 72, 153, 255}, {126, 34, 206, 255}},
    // Theme 2: Forest (green tones)
    {{16, 185, 129, 255}, {5, 150, 105, 255}, {4, 120, 87, 255}},
    // Theme 3: Cyber (purple/magenta)
    {{168, 85, 247, 255}, {217, 70, 239, 255}, {79, 70, 229, 255}},
    // Theme 4: Midnight (dark blues)
    {{30, 41, 59, 255}, {15, 23, 42, 255}, {2, 6, 23, 255}}
};


// ============================================================================
// SECTION 2: GAME SCREENS (States)
// ============================================================================
// The game can be in one of these screens at any time.
// We use an "enum" to give meaningful names to each screen.

enum Screen {
    MENU,   // Main menu with buttons
    NAME,   // Player name entry screen
    DIFF,   // Difficulty selection (Easy/Hard)
    MUSIC,  // Music selection screen
    THEME,  // Theme/color selection screen
    GAME,   // The actual game where you guess words
    LEAD    // Leaderboard showing high scores
};

Screen scr = MENU;  // Start at the main menu


// ============================================================================
// SECTION 3: DATA STRUCTURES
// ============================================================================
// These are custom "types" that group related data together.
// Think of them like templates for creating objects with multiple properties.

// Tile: Represents one letter box on the game board
struct Tile {
    char l = ' ';     // The letter displayed (space means empty)
    int s = 0;        // State: 0=empty, 1=yellow, 2=green, 3=gray
    bool rev = false; // Has the flip animation revealed the color?
    float t = 0;      // Time when the tile should start flipping
};

// Particle: Used for the celebration effect when you win
struct Particle {
    float x, y;    // Position on screen
    float vx, vy;  // Velocity (speed in x and y directions)
    float life;    // How long until this particle disappears (0 to 1)
    Color c;       // Color of the particle
};


// ============================================================================
// SECTION 4: GLOBAL VARIABLES
// ============================================================================
// These variables hold the current state of the game and are accessible
// from anywhere in the code.

// --- Game Board ---
Tile grid[ROWS][COLS];      // 2D array of tiles (6 rows x 5 columns)
vector<Particle> parts;     // List of active celebration particles
vector<string> words;       // All valid 5-letter words loaded from file

// --- Player Data ---
string pName;               // Player's name (entered at start)
string ans;                 // The answer word to guess (randomly chosen)

// --- Game State ---
int curR = 0;               // Current row (which guess we're on, 0-5)
int curC = 0;               // Current column (which letter position, 0-4)
int lStates[26] = {0};      // Keyboard letter states (index 0='A', 1='B', etc.)
                            // 0=unused, 1=yellow, 2=green, 3=gray

// --- Settings ---
int theme = 0;              // Currently selected theme (0-4)
int diff = 0;               // Difficulty: 0=Easy, 1=Hard
int lvl = 1;                // Current level/round number
int score = 0;              // Player's total score
int musId = 0;              // Currently playing music track (0-2)

// --- Game Flags ---
bool over = false;          // Is the current game finished?
bool won = false;           // Did the player win?
bool aud = false;           // Is audio available/enabled?
bool muted = false;         // Track if game is muted
float scrl = 0;             // Scroll position for leaderboard

// --- Raylib Resources ---
Font font;                  // Custom font for text rendering
Music mus[3];               // Array of 3 music tracks
Sound sType;                // Typing sound effect
Sound sWin;                 // Victory sound effect
Sound sFail;                // Failure sound effect


// ============================================================================
// SECTION 5: HELPER FUNCTIONS
// ============================================================================
// Small utility functions that are used throughout the code.

// ----------------------------------------------------------------------------
// Txt: Draw centered text at position (x, y)
// Parameters:
//   t = the text to display
//   x, y = center position for the text
//   s = font size
//   c = text color
// ----------------------------------------------------------------------------
void Txt(string t, float x, float y, int s, Color c) {
    // MeasureTextEx calculates how wide and tall the text will be
    Vector2 d = MeasureTextEx(font, t.c_str(), s, 1);
    
    // Draw the text offset by half its size so it's centered at (x, y)
    DrawTextEx(font, t.c_str(), {x - d.x / 2, y - d.y / 2}, s, 1, c);
}

// ----------------------------------------------------------------------------
// Btn: Draw a button and check if it was clicked
// Parameters:
//   t = button label text
//   x, y = top-left position
//   w, h = width and height
//   c = button background color
// Returns: true if the button was clicked this frame
// ----------------------------------------------------------------------------
bool Btn(string t, float x, float y, float w, float h, Color c) {
    // Check if mouse is hovering over the button
    bool hv = CheckCollisionPointRec(GetMousePosition(), {x, y, w, h});
    
    // Draw button rectangle (slightly raised if hovered)
    float yOffset = hv ? 2 : 0;
    Color bgColor = hv ? ColorBrightness(c, 0.3f) : c;
    DrawRectangle(x, y - yOffset, w, h, bgColor);
    
    // Draw border around button
    Color borderColor = ColorBrightness(c, -0.2f);
    DrawRectangleLinesEx({x, y - yOffset, w, h}, 2, borderColor);
    
    // Draw centered label text
    Txt(t, x + w / 2, y + h / 2 - yOffset, 20, WHITE);
    
    // Return true if button was clicked
    return hv && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

// ----------------------------------------------------------------------------
// SpawnPart: Create celebration particles at position (x, y)
// Called when the player wins to create the confetti effect
// ----------------------------------------------------------------------------
void SpawnPart(float x, float y) {
    // Create 15 particles shooting in random directions
    for (int i = 0; i < 15; i++) {
        Particle p;
        p.x = x;
        p.y = y;
        
        // Random angle converted to velocity
        float angle = GetRandomValue(0, 360) * DEG2RAD;
        p.vx = cosf(angle) * 3;           // Horizontal speed
        p.vy = sinf(angle) * 3 - 2;       // Vertical speed (with upward boost)
        
        p.life = 1.0f;   // Start with full life
        p.c = C_GRN;     // Green color
        
        parts.push_back(p);  // Add to particle list
    }
}


// ============================================================================
// SECTION 6: FILE HANDLING FUNCTIONS
// ============================================================================
// Functions for loading words and saving/loading the leaderboard.

// ----------------------------------------------------------------------------
// Load: Read all 5-letter words from a file
// Parameter: f = filename to load
// Returns: Vector of words (all uppercase)
// ----------------------------------------------------------------------------
vector<string> Load(string f) {
    vector<string> l;       // List to store words
    ifstream i(f);          // Open file for reading
    string s;               // Temporary string for each word
    
    // Read each word from file
    while (i >> s) {
        // Only accept 5-letter words
        if (s.size() == 5) {
            // Convert to uppercase
            for (char &c : s) {
                c = toupper(c);
            }
            l.push_back(s);
        }
    }
    
    return l;
}

// ----------------------------------------------------------------------------
// Save: Save score to leaderboard file
// Parameter: p = points earned this round
// ----------------------------------------------------------------------------
void Save(int p) {
    score += p;  // Add points to total score
    
    // Load existing leaderboard entries
    vector<pair<string, int>> s;  // List of (name, score) pairs
    ifstream f("assets/leaderboard.txt");
    string l;
    bool fnd = 0;  // Did we find this player already?
    
    // Read each line from leaderboard
    while (getline(f, l)) {
        size_t c = l.find(',');  // Find the comma separator
        if (c != -1) {
            string name = l.substr(0, c);
            int scr = stoi(l.substr(c + 1));
            s.push_back({name, scr});
        }
    }
    
    // Update existing player or add new entry
    for (auto &x : s) {
        if (x.first == pName) {
            x.second += p;  // Update existing score
            fnd = 1;
            break;
        }
    }
    if (!fnd) {
        s.push_back({pName, score});  // Add new player
    }
    
    // Write updated leaderboard back to file
    ofstream o("assets/leaderboard.txt");
    for (auto &x : s) {
        o << x.first << "," << x.second << "\n";
    }
}

// ----------------------------------------------------------------------------
// Reset: Start a new game round
// Clears the board and picks a new random word
// ----------------------------------------------------------------------------
void Reset() {
    // Don't reset if no words were loaded
    if (words.empty()) return;
    
    // Pick a random word as the answer
    ans = words[GetRandomValue(0, words.size() - 1)];
    
    // Reset game state
    curR = 0;
    curC = 0;
    over = false;
    won = false;
    parts.clear();
    
    // Clear all tiles on the board
    for (auto &r : grid) {
        for (auto &t : r) {
            t = {' ', 0, 0, 0};  // Empty tile
        }
    }
    
    // Reset keyboard letter colors
    memset(lStates, 0, sizeof(lStates));
}


// ============================================================================
// SECTION 7: MAIN FUNCTION - PROGRAM ENTRY POINT
// ============================================================================

int main() {
    // ------------------------------------------------------------------------
    // INITIALIZATION
    // ------------------------------------------------------------------------
    
    // Create the game window (1000 pixels wide, 920 pixels tall)
    InitWindow(1000, 920, "Wordle++");
    SetTargetFPS(60);  // Limit to 60 frames per second for smooth animation
    
    // Initialize audio system for music and sound effects
    InitAudioDevice();
    
    // Load custom font for nice-looking text
    font = LoadFontEx("fonts/Anton-Regular.ttf", 64, 0, 0);
    
    // Load the word list
    words = Load("assets/words.txt");
    
    // Try to load audio files (they might not exist)
    if (FileExists("assets/track1.mp3")) {
        // Load all 3 music tracks
        for (int i = 0; i < 3; i++) {
            string filename = "assets/track" + to_string(i + 1) + ".mp3";
            mus[i] = LoadMusicStream(filename.c_str());
        }
        
        // Load sound effects
        sType = LoadSound("assets/type.wav");
        sWin = LoadSound("assets/success.wav");
        sFail = LoadSound("assets/fail.wav");
        
        // Start playing the first music track
        aud = true;
        PlayMusicStream(mus[0]);
        SetMusicVolume(mus[0], 0.3f);  // 30% volume
    }

    // ------------------------------------------------------------------------
    // MAIN GAME LOOP
    // Runs once per frame until the window is closed
    // ------------------------------------------------------------------------
    
    while (!WindowShouldClose()) {
        
        // --------------------------------------------------------------------
        // UPDATE: Process game logic before drawing
        // --------------------------------------------------------------------
        
        // Keep music playing (must be called every frame)
        if (aud) {
            UpdateMusicStream(mus[musId]);
        }
        
        // Update celebration particles (move them and remove dead ones)
        for (auto i = parts.begin(); i != parts.end();) {
            i->x += i->vx;        // Move horizontally
            i->y += i->vy;        // Move vertically
            i->vy += 0.15f;       // Apply gravity
            i->life -= 0.02f;     // Decrease life
            
            // Remove particles that have faded away
            if (i->life <= 0) {
                i = parts.erase(i);
            } else {
                i++;
            }
        }

        // --------------------------------------------------------------------
        // DRAW: Render everything to the screen
        // --------------------------------------------------------------------
        
        BeginDrawing();
        
        // Draw gradient background using current theme colors
        Color *bg = (Color *)THEMES[theme];
        DrawRectangleGradientV(0, 0, 1000, 460, ColorAlpha(bg[0], 0.9f), ColorAlpha(bg[1], 0.9f));
        DrawRectangleGradientV(0, 460, 1000, 460, ColorAlpha(bg[1], 0.9f), ColorAlpha(bg[2], 0.9f));
        
        // Draw decorative animated wave circles in background
        for (int i = 0; i < 3; i++) {
            for (int x = 0; x < 1000; x += 20) {
                float waveY = 300 + i * 150 + sinf(x * 0.01f + GetTime() * 0.2f + i) * 30;
                DrawCircle(x, waveY, 8, ColorAlpha(WHITE, 0.05f));
            }
        }
        
        // Draw game title with subtle bounce animation
        float titleY = 40 + sinf(GetTime() * 2) * 3;
        Txt("WORDLE++", 500, titleY, 48, WHITE);

        // ====================================================================
        // SCREEN-SPECIFIC RENDERING
        // ====================================================================
        
        // --------------------------------------------------------------------
        // MENU SCREEN
        // --------------------------------------------------------------------
        if (scr == MENU) {
            // Display current level and score
            Txt("Level: " + to_string(lvl), 500, 260, 24, C_YEL);
            Txt("Score: " + to_string(score), 500, 290, 24, C_YEL);
            
            // Menu buttons
            if (Btn("Start Game", 350, 350, 300, 50, C_GRN)) {
                scr = NAME;
            }
            if (Btn("Difficulty", 350, 420, 300, 50, C_ACC)) {
                scr = DIFF;
            }
            if (Btn("Music", 350, 490, 300, 50, C_ACC)) {
                scr = MUSIC;
            }
            if (Btn("Theme", 350, 560, 300, 50, C_ACC)) {
                scr = THEME;
            }
            if (Btn("Leaderboard", 350, 630, 300, 50, C_YEL)) {
                scr = LEAD;
                scrl = 0;  // Reset scroll position
            }
            if (Btn("Exit", 350, 700, 300, 50, C_GRY)) {
                break;  // Exit the game loop (cleanup happens after loop)
            }
        }
        
        // --------------------------------------------------------------------
        // NAME ENTRY SCREEN
        // --------------------------------------------------------------------
        else if (scr == NAME) {
            // Handle keyboard input for name
            int k = GetCharPressed();
            while (k) {
                // Add printable characters (limit 12 characters)
                if (k >= 32 && k <= 125 && pName.size() < 12) {
                    pName += (char)k;
                    if (aud) PlaySound(sType);
                }
                k = GetCharPressed();
            }
            
            // Handle backspace to delete characters
            if (IsKeyPressed(KEY_BACKSPACE) && !pName.empty()) {
                pName.pop_back();
                if (aud) PlaySound(sType);
            }
            
            // Handle enter to confirm name and start game
            if (IsKeyPressed(KEY_ENTER) && !pName.empty()) {
                // Convert name to uppercase
                for (char &c : pName) {
                    c = toupper(c);
                }
                Reset();      // Start new game
                scr = GAME;   // Go to game screen
                if (aud) PlaySound(sWin);
            }
            
            // Display name with blinking cursor
            string cursor = (int(GetTime() * 2) % 2) ? "|" : "";
            Txt("Name: " + pName + cursor, 500, 355, 30, WHITE);
            
            // Back button
            if (Btn("Back", 430, 500, 140, 40, C_GRY)) {
                scr = MENU;
            }
        }
        
        // --------------------------------------------------------------------
        // DIFFICULTY SELECTION SCREEN
        // --------------------------------------------------------------------
        else if (scr == DIFF) {
            // Check hover state for each difficulty option
            bool easyHover = CheckCollisionPointRec(GetMousePosition(), {250, 300, 200, 100});
            bool hardHover = CheckCollisionPointRec(GetMousePosition(), {550, 300, 200, 100});
            
            // Draw EASY button
            Color easyColor = easyHover ? ColorBrightness(C_GRN, 0.3f) : C_GRN;
            DrawRectangle(250, 300, 200, 100, easyColor);
            Color easyBorder = (diff == 0) ? C_YEL : WHITE;  // Highlight if selected
            DrawRectangleLinesEx({250, 300, 200, 100}, 3, easyBorder);
            Txt("EASY", 350, 330, 28, WHITE);
            
            // Draw HARD button
            Color hardColor = hardHover ? ColorBrightness(RED, 0.3f) : RED;
            DrawRectangle(550, 300, 200, 100, hardColor);
            Color hardBorder = (diff == 1) ? C_YEL : WHITE;  // Highlight if selected
            DrawRectangleLinesEx({550, 300, 200, 100}, 3, hardBorder);
            Txt("HARD", 650, 330, 28, WHITE);
            
            // Handle click to select difficulty
            if ((easyHover || hardHover) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                diff = hardHover ? 1 : 0;
                if (aud) PlaySound(sType);
            }
            
            // Back button
            if (Btn("Back", 430, 500, 140, 40, C_GRY)) {
                scr = MENU;
            }
        }
        
        // --------------------------------------------------------------------
        // MUSIC SELECTION SCREEN
        // --------------------------------------------------------------------
        else if (scr == MUSIC) {
            string tr[] = {"Chill", "Epic", "LoFi"};
            
            // Draw a button for each music track
            for (int i = 0; i < 3; i++) {
                Color btnColor = (musId == i) ? C_YEL : C_ACC;
                
                if (Btn(tr[i], 300, 280 + i * 80, 400, 60, btnColor) && aud) {
                    // Switch to new track
                    StopMusicStream(mus[musId]);
                    musId = i;
                    PlayMusicStream(mus[i]);
                    SetMusicVolume(mus[i], 0.3f);
                    PlaySound(sType);
                }
            }
            
            // Back button
            if (Btn("Back", 430, 600, 140, 40, C_GRY)) {
                scr = MENU;
            }
        }
        
        // --------------------------------------------------------------------
        // THEME SELECTION SCREEN
        // --------------------------------------------------------------------
        else if (scr == THEME) {
            string th[] = {"Ocean", "Sunset", "Forest", "Cyber", "Midnight"};
            
            // Draw a button for each theme
            for (int i = 0; i < 5; i++) {
                Color btnColor = (theme == i) ? C_YEL : THEMES[i][0];
                
                if (Btn(th[i], 250, 220 + i * 70, 500, 55, btnColor)) {
                    theme = i;
                    if (aud) PlaySound(sType);
                }
            }
            
            // Back button
            if (Btn("Back", 430, 750, 140, 40, C_GRY)) {
                scr = MENU;
            }
        }
        
        // --------------------------------------------------------------------
        // GAME SCREEN - Main gameplay
        // --------------------------------------------------------------------
        else if (scr == GAME) {
            // Display current level and difficulty
            string diffText = diff ? "HARD" : "EASY";
            Txt("Lvl " + to_string(lvl) + " | " + diffText, 500, 90, 20, WHITE);
            
            // --- MUTE BUTTON ---
            if (Btn(muted ? "UNMUTE" : "MUTE", 820, 80, 120, 40, muted ? RED : C_GRN)) {
                muted = !muted;
                SetMasterVolume(muted ? 0.0f : 1.0f);
            }
            
            // ----------------------------------------------------------------
            // INPUT HANDLING (only when game is not over)
            // ----------------------------------------------------------------
            if (!over) {
                // Handle letter input
                int k = GetCharPressed();
                if (isalpha(k) && curC < 5) {
                    grid[curR][curC].l = (char)toupper(k);
                    grid[curR][curC].s = 0;
                    grid[curR][curC].rev = false;
                    grid[curR][curC].t = (float)GetTime();
                    curC++;
                    if (aud) PlaySound(sType);
                }
                
                // Handle backspace
                if (IsKeyPressed(KEY_BACKSPACE) && curC > 0) {
                    curC--;
                    grid[curR][curC].l = ' ';
                    if (aud) PlaySound(sType);
                }
                
                // Handle arrow key navigation
                if (IsKeyPressed(KEY_LEFT) && curC > 0) {
                    curC--;
                }
                if (IsKeyPressed(KEY_RIGHT) && curC < 5 && grid[curR][curC].l != ' ') {
                    curC++;
                }
                
                // Handle Enter to submit guess
                if (IsKeyPressed(KEY_ENTER) && curC == 5) {
                    // Build the guessed word
                    string g;
                    for (int i = 0; i < 5; i++) {
                        g += grid[curR][i].l;
                    }
                    
                    // Count letters in answer for matching algorithm
                    // here c is the ascii value of A which is 65
                    int cnt[26] = {0};
                    for (char c : ans) {
                        cnt[c - 'A']++;
                    }
                    
                    // First pass: Mark exact matches (GREEN)
                    for (int i = 0; i < 5; i++) {
                        if (g[i] == ans[i]) {
                            grid[curR][i].s = 2;  // GREEN
                            cnt[g[i] - 'A']--;    // Use up this letter
                            lStates[g[i] - 'A'] = 2;  // Mark on keyboard
                        }
                    }
                    
                    // Second pass: Mark partial matches (YELLOW) and misses (GRAY)
                    for (int i = 0; i < 5; i++) {
                        if (grid[curR][i].s != 2) {  // Not already green
                            int idx = g[i] - 'A';
                            
                            if (cnt[idx] > 0) {
                                // Letter exists elsewhere in word
                                grid[curR][i].s = 1;  // YELLOW
                                cnt[idx]--;
                                if (lStates[idx] != 2) {
                                    lStates[idx] = 1;
                                }
                            } else {
                                // Letter not in word (or already used up)
                                grid[curR][i].s = 3;  // GRAY
                                if (!lStates[idx]) {
                                    lStates[idx] = 3;
                                }
                            }
                        }
                    }
                    
                    // Set flip animation timing
                    for (int i = 0; i < 5; i++) {
                        grid[curR][i].t = GetTime() + i * 0.1f;  // Stagger the flips
                    }
                    
                    // Check win condition
                    if (g == ans) {
                        over = true;
                        won = true;
                        
                        // Calculate score: base points + row bonus
                        int points = (diff ? 20 : 10) + (ROWS - curR) * 5;
                        Save(points);
                        
                        lvl++;  // Advance to next level
                        if (aud) PlaySound(sWin);
                        
                        // Spawn celebration particles
                        for (int i = 0; i < 5; i++) {
                            SpawnPart(320 + i * 90, 160 + curR * 90);
                        }
                    } 
                    // Check lose condition (used all rows)
                    else if (++curR == ROWS) {
                        over = true;
                        Save(0);  // No points for losing
                        if (aud) PlaySound(sFail);
                    } 
                    // Continue to next row
                    else {
                        curC = 0;  // Reset column for new row
                    }
                }
            }
            
            // ----------------------------------------------------------------
            // DRAW GAME BOARD
            // ----------------------------------------------------------------
            for (int r = 0; r < ROWS; r++) {
                for (int c = 0; c < COLS; c++) {
                    Tile &t = grid[r][c];
                    
                    // Calculate tile position
                    float x = 280 + c * 90;
                    float y = 140 + r * 90;
                    float sy = 1.0f;  // Scale for flip animation
                    
                    // Default tile colors
                    Color col = {30, 30, 35, 255};  // Dark gray background
                    Color out = C_ACC;              // Purple outline
                    
                    // Animate tiles that have been submitted
                    if (r < curR || (over && r == curR)) {
                        float dt = (GetTime() - t.t) / 0.5f;  // Animation progress
                        
                        // Flip animation: squish down, then expand with color
                        if (dt < 0.5f) {
                            sy = 1 - dt * 2;  // Squish phase
                        } else {
                            sy = (dt - 0.5f) * 2;  // Expand phase
                            t.rev = true;  // Show color now
                        }
                        
                        // Set tile color based on state
                        if (t.rev) {
                            if (t.s == 2) col = C_GRN;       // GREEN
                            else if (t.s == 1) col = C_YEL;  // YELLOW
                            else col = C_GRY;                // GRAY
                            out = col;
                        }
                        
                        // Clamp animation
                        if (dt > 1) sy = 1;
                    }
                    
                    // Draw the tile rectangle (with flip animation)
                    DrawRectangleRec({x, y + 40 * (1 - sy), 80, 80 * sy}, col);
                    
                    // Draw tile border (thicker and white if cursor is here)
                    bool act = (!over && r == curR && c == curC);
                    Color borderColor;
                    if (act) {
                        borderColor = WHITE;
                    } else if (!t.rev && r == curR && c == curC && !over) {
                        borderColor = C_YEL;
                    } else {
                        borderColor = out;
                    }
                    DrawRectangleLinesEx({x, y + 40 * (1 - sy), 80, 80 * sy}, act ? 4 : 2, borderColor);
                    
                    // Draw the letter
                    if (t.l != ' ') {
                        Txt(string(1, t.l), x + 40, y + 40, 36, WHITE);
                    }
                }
            }
            
            // ----------------------------------------------------------------
            // DRAW ON-SCREEN KEYBOARD
            // ----------------------------------------------------------------
            string ks[] = {"QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM"};
            
            for (int r = 0; r < 3; r++) {
                // Center the row
                float x = (1000 - ks[r].size() * 45) / 2;
                
                for (char k : ks[r]) {
                    int s = lStates[k - 'A'];
                    
                    // Determine key color based on state
                    Color keyColor;
                    if (s == 2) keyColor = C_GRN;       // GREEN - correct position
                    else if (s == 1) keyColor = C_YEL; // YELLOW - wrong position
                    else if (s == 3) keyColor = C_GRY; // GRAY - not in word
                    else keyColor = {50, 50, 60, 255}; // Unused - dark gray
                    
                    DrawRectangle(x, 720 + r * 50, 40, 40, keyColor);
                    Txt(string(1, k), x + 20, 740 + r * 50, 20, WHITE);
                    x += 45;
                }
            }
            
            // ----------------------------------------------------------------
            // GAME OVER UI
            // ----------------------------------------------------------------
            if (over) {
                // Show result message
                if (won) {
                    Txt("VICTORY", 500, 110, 30, C_GRN);
                } else {
                    Txt("FAILED: " + ans, 500, 110, 30, RED);
                }
                
                // Action buttons
                if (Btn("Menu", 200, 870, 120, 40, C_GRY)) {
                    scr = MENU;
                }
                
                string nextBtnText = won ? "Next" : "Retry";
                Color nextBtnColor = won ? C_GRN : C_YEL;
                if (Btn(nextBtnText, 420, 870, 160, 40, nextBtnColor)) {
                    Reset();
                }
                
                if (Btn("Exit", 680, 870, 120, 40, C_GRY)) {
                    break;  // Exit the game loop (cleanup happens after loop)
                }
            }
            
            // Draw celebration particles
            for (auto &p : parts) {
                DrawCircle(p.x, p.y, 3, Fade(p.c, p.life));
            }
        }
        
        // --------------------------------------------------------------------
        // LEADERBOARD SCREEN
        // --------------------------------------------------------------------
        else if (scr == LEAD) {
            Txt("HALL OF FAME", 500, 100, 30, WHITE);
            
            // Load leaderboard entries from file
            vector<string> ls;
            ifstream f("assets/leaderboard.txt");
            string l;
            while (getline(f, l)) {
                ls.push_back(l);
            }
            
            // Sort by score (highest first)
            sort(ls.begin(), ls.end(), [](string a, string b) {
                int scoreA = stoi(a.substr(a.find(',') + 1));
                int scoreB = stoi(b.substr(b.find(',') + 1));
                return scoreA > scoreB;
            });
            
            // Handle scrolling
            scrl -= GetMouseWheelMove() * 20;
            if (scrl < 0) scrl = 0;
            
            // Draw entries with clipping (only show within the view area)
            BeginScissorMode(200, 140, 600, 500);
            for (int i = 0; i < ls.size(); i++) {
                // Format: replace comma with " - "
                string entry = ls[i];
                entry.replace(entry.find(','), 1, " - ");
                
                Txt(to_string(i + 1) + ". " + entry, 500, 160 + i * 35 - scrl, 20, WHITE);
            }
            EndScissorMode();
            
            // Back button
            if (Btn("Back", 430, 700, 140, 40, C_GRY)) {
                scr = MENU;
            }
        }
        
        EndDrawing();
    }

    // ------------------------------------------------------------------------
    // CLEANUP
    // Clean up resources before exiting
    // ------------------------------------------------------------------------
    CloseAudioDevice();
    UnloadFont(font);
    CloseWindow();
    
    return 0;
}