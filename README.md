# Bookworm — C++ / SFML Word Game
### College Object-Oriented Programming Project

A recreation of the classic **Bookworm** word game built entirely in C++ with an SFML graphical interface. Spell words from adjacent tiles, manage burning tiles before they explode, and climb the leaderboard!

---

## Project Structure

```
bookworm_project/
├── include/
│   ├── Tile.h          # Single letter tile — state, burn counter, animation
│   ├── Grid.h          # 8×8 board, burning tile logic, DFS word checker
│   ├── Dictionary.h    # Word validity checker (O(1) hash set lookup)
│   ├── Player.h        # Score, XP bar, level, lives (hearts)
│   └── GameEngine.h    # Top-level orchestrator, UI, buttons, game loop
├── src/
│   ├── main.cpp        # Entry point
│   ├── Tile.cpp
│   ├── Grid.cpp
│   ├── Dictionary.cpp
│   ├── Player.cpp
│   └── GameEngine.cpp
├── assets/
│   ├── fonts/          # Place your .ttf font file here
│   ├── music/          # Background music (.mp3)
│   ├── sfx/            # Sound effects (.mp3)
│   ├── words.txt       # Optional: one English word per line
│   └── highscore.txt   # Auto-generated high score file
├── CMakeLists.txt
└── README.md
```

---

## 🛠 Required Tools & Versions

| Tool | Version | Download |
|------|---------|---------|
| **MinGW-w64** (g++) | GCC 14.3.0 UCRT POSIX | https://winlibs.com → `winlibs-x86_64-posix-seh-gcc-14.3.0-mingw-w64ucrt-12.0.0-r1.7z` |
| **CMake** | 3.28+ | https://cmake.org/download → `cmake-3.31.x-windows-x86_64.msi` |
| **SFML** | 3.0.0 | Auto-downloaded by CMake — no manual install needed |
| **Git** | Any | https://git-scm.com — needed by CMake FetchContent |
| **VS Code** | Latest | https://code.visualstudio.com |

### VS Code Extensions Required

| Extension | Publisher |
|-----------|-----------|
| C/C++ | Microsoft (ms-vscode.cpptools) |
| CMake Tools | Microsoft (ms-vscode.cmake-tools) |

---

## Installation & Setup

### Step 1 — Install MinGW-w64

1. Go to https://winlibs.com
2. Download: **GCC 14.3.0 + MinGW-w64 12.0.0 (UCRT) — Win64 — without LLVM — 7-Zip archive**
3. Extract the archive to `C:\mingw64`
4. Add `C:\mingw64\bin` to your Windows PATH:
   - Search → **"Edit the system environment variables"**
   - Click **Environment Variables** → select **Path** → **Edit**
   - Click **New** → type `C:\mingw64\bin` → OK all dialogs
5. Verify in a **new** terminal:
   ```
   g++ --version
   ```
   Expected: `g++ (GCC) 14.3.0`

### Step 2 — Install CMake

1. Go to https://cmake.org/download
2. Download the **stable** Windows x64 Installer `.msi` (not the RC release candidate)
3. During installation, check **"Add CMake to the system PATH for all users"**
4. Verify:
   ```
   cmake --version
   ```
   Expected: `cmake version 3.28.x` or higher

### Step 3 — Install Git

CMake's FetchContent needs Git to download SFML automatically.

1. Go to https://git-scm.com/download/win
2. Install with default settings
3. Verify:
   ```
   git --version
   ```

### Step 4 — Install VS Code and Extensions

1. Download VS Code from https://code.visualstudio.com
2. Open VS Code and press `Ctrl+Shift+X`
3. Search and install:
   - **C/C++** by Microsoft
   - **CMake Tools** by Microsoft

### Step 5 — Add a Font File

The game needs a TrueType font (`.ttf`).

**Option A — Use Windows built-in Arial (easiest):**
The game automatically looks for `C:\Windows\Fonts\arial.ttf`. No action needed.

**Option B — Add a custom font:**
1. Download Roboto from https://fonts.google.com/specimen/Roboto
2. Place `Roboto-Regular.ttf` inside `bookworm_project/assets/fonts/`

### Step 6 — (Optional) Add a Word List

A large dictionary significantly improves gameplay variety.

1. Download `words_alpha.txt` from https://github.com/dwyl/english-words
2. Rename it to `words.txt`
3. Place it at `bookworm_project/assets/words.txt`

If this file is missing, the game automatically falls back to a built-in list of common English words.

---

## How to Compile, Build, and Run

### Option A — Command Prompt (CMD)

Open CMD and navigate to the project folder:

```cmd
cd C:\path\to\bookworm_project
```

Then run:

```cmd
rmdir /s /q build
mkdir assets\fonts
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
.\build\Bookworm.exe
```

> **First build takes 2–5 minutes** — CMake downloads and compiles SFML 3.0.0 automatically. Subsequent builds are fast.

### Option B — VS Code

1. Open VS Code
2. **File → Open Folder** → select the `bookworm_project` folder
3. When prompted "Would you like to configure this project?" click **Yes**
   - If not prompted: `Ctrl+Shift+P` → **CMake: Configure**
4. Select compiler kit: choose **GCC 14.x.x (MinGW)**
5. Build: press `Ctrl+Shift+B`
6. Run from the integrated terminal:
   ```
   .\build\Bookworm.exe
   ```

### Option C — PowerShell

```powershell
cd C:\path\to\bookworm_project
Remove-Item -Recurse -Force build
mkdir assets\fonts -Force
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
.\build\Bookworm.exe
```

---

## How to Play

An 8×8 grid of letter tiles is displayed. Spell valid English words by clicking adjacent tiles and submit them to score points. Watch out for burning tiles!

### Controls

| Control | Action |
|---------|--------|
| **Left-click** a tile | Select it (must be adjacent to the last selected tile) |
| **Left-click** the last tile again | Deselect it (undo last letter) |
| **Enter** | Submit the current word |
| **Backspace** | Clear the entire selection |
| **R** | Restart the game |
| **Submit button** | Submit the current word |
| **Clear button** | Clear the entire selection |
| **New Game button** | Start a fresh game |
| **Exit button** | Return to the main menu |
| **? button** | Open the How to Play screen |

### Rules

- Words must be **at least 3 letters** long
- Each tile selected must be **8-directionally adjacent** (horizontal, vertical, or diagonal) to the previous tile
- You cannot use the same tile twice in one word
- The word must exist in the dictionary

### Scoring Formula

```
base  = sum of letter point values for each tile used
bonus = word length × 10   (5-letter words)
      = word length × 20   (6–7 letter words)
      = word length × 50   (8+ letter words)
total = (base + bonus) × current level
```

Letter values follow the Scrabble scale — A=1, E=1, Z=10, Q=10, etc.

### Level Progression

- Every valid word earns **XP equal to its point value**
- The **XP bar** fills as you score points — when full, you level up
- Each level **increases the score multiplier**
- Levelling up **spawns a burning tile** on the board

---

## Burning Tiles

### What They Are
Burning tiles are letter blocks displayed with an animated pulsing red/orange colour and a countdown number showing how many words remain before they explode.

### How the Counter Works
- Every burning tile starts with a counter of **3**
- Each time you submit a valid word, all burning tiles tick down by 1
- When a tile reaches **0** it explodes

### What Happens When They Explode
- You **lose 1 life** (heart)
- The exploded tile is replaced with a fresh normal tile
- If you run out of all 5 lives, the game ends

### How to Remove Burning Tiles
Use the burning tile's letter in a valid word. When the word is accepted and those tiles are removed from the board, the burning tile disappears with them.

### Spawn Rules
- Burning tiles only spawn from **Level 3+** (random ignition) and **Level 4+** (column refill)
- The board **never has more than 2 burning tiles** at once
- Burning tiles are only placed where at least one adjacent tile is a vowel — ensuring they can always be used in a word
- Burning tiles only spawn in the **top 6 rows** so they always have neighbours

---

## Lives System

- You start with **5 lives** shown as red heart icons in the HUD
- A life is lost each time a burning tile explodes
- When all 5 lives are gone the game ends and your score is saved

---

## High Score

- Your highest score is automatically saved to `assets/highscore.txt`
- The high score is displayed on the main menu screen
- It persists between sessions

---

## Architecture Overview

```
main()
  └── GameEngine::run()
        ├── processEvents()
        │     ├── onMouseMoved()    → button hover updates
        │     ├── onMousePressed()  → buttons + Grid::onMousePressed()
        │     └── onKeyPressed()    → submit / clear / restart
        │
        ├── update(dt)
        │     ├── Grid::hasExploded()    → lose life, clear tile
        │     ├── Player::isAlive()      → game over check
        │     └── Grid::hasPossibleWord() → warning banner
        │
        └── render()
              ├── renderMenu()           → title screen
              ├── renderGame()
              │     ├── Grid::draw()
              │     │     └── Tile::draw(elapsed) → burn pulse animation
              │     ├── drawSidebar()    → hearts, XP bar, score, level
              │     └── toolbar buttons
              ├── renderGameOverOverlay()
              └── renderHelp()           → how to play screen
```

### Class Responsibilities

| Class | Responsibility |
|-------|---------------|
| **Tile** | One letter cell. Stores letter, grid position, burn counter, and state (Normal / Selected / Burning). Draws itself with animated burn colour. `tickBurn()` decrements counter per word submitted. |
| **Grid** | Owns all 64 Tiles on the 8×8 board. Enforces adjacency selection rules. On word submission: removes tiles, rebuilds columns, ticks burn counters, and handles random ignition. `hasPossibleWord()` runs a DFS to warn when no word is possible. |
| **Dictionary** | Loads a word list into `unordered_set` for O(1) lookups. Falls back to a built-in word list automatically. |
| **Player** | Tracks score, XP, level, and lives. `addWord()` awards points and XP. `checkLevelUp()` advances the level when XP threshold is met. `loseLife()` decrements hearts. |
| **GameEngine** | Top-level orchestrator. Owns all subsystems. Manages game states (Menu / Playing / GameOver / Help). Handles all SFML events, button interactions, audio, high score I/O, and the render pipeline. |
| **Button** | Reusable clickable UI element with hover colour effect. Uses `std::optional<sf::Text>` to work around SFML 3's deleted default constructor. |

### Data Flow (word submission)

```
Player clicks tiles   →  Grid::onMousePressed() records selection
                         (adjacency + duplicate rules enforced)

Player presses ENTER  →  GameEngine::trySubmitWord()
                          → Grid::getSelectedWord()
                          → Dictionary::isValid(word)
                                YES → Player::addWord(word, tileSum)
                                      Grid::removeSelectedTiles(level)
                                      [level up?] → spawnBurningTile(1)
                                      showMessage("Nice word! +pts")
                                NO  → showMessage("Not a valid word!")
                                      Grid::clearSelection()
```

---

## Common Issues

| Problem | Solution |
|---------|----------|
| `cmake` not found | Re-install CMake and make sure "Add to PATH" was ticked during install |
| `g++` not found | Check that `C:\mingw64\bin` is in your PATH; restart terminal after adding |
| Font not loading / blank window | Place `arial.ttf` next to `Bookworm.exe`, or add a `.ttf` to `assets/fonts/` |
| SFML download fails | Ensure Git is installed and internet is available during first build |
| `ninja` generator error | Always use `-G "MinGW Makefiles"` explicitly |
| CMake policy warning | The `set(CMAKE_POLICY_VERSION_MINIMUM "3.5")` in CMakeLists.txt handles this automatically |
| No sound | Ensure the `assets/music/` and `assets/sfx/` folders exist with the `.mp3` files |
| Assets not copying | Run `mkdir assets\fonts` before the first build |

---

## Team Contributions

| Member | Responsibilities |
|--------|-----------------|
| **Member A** | Tile burn mechanics, 8×8 grid, `fillColumn` survivor logic, burn spawn rules, `hasExploded`, `clearExplodedTiles` |
| **Member B** | Button struct, toolbar UI, menu screen, pause/game over overlays, DFS possible-word checker, `hasPossibleWord` |
| **Member C** | Player lives (hearts), XP bar system, score/high score, sidebar HUD, background music, sound effects |