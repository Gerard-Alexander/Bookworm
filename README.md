# Bookworm — C++ / SFML Word Game
### College Object-Oriented Programming Project

A recreation of the classic **Bookworm** word game built entirely in C++ with an SFML graphical interface.

---

## Project Structure

```
bookworm_project/
├── include/
│   ├── Tile.h          # Single letter tile on the grid
│   ├── Grid.h          # 4×4 board of tiles
│   ├── Dictionary.h    # Word validity checker
│   ├── Player.h        # Score tracking and level progression
│   └── GameEngine.h    # Top-level orchestrator + game loop
├── src/
│   ├── main.cpp        # Entry point
│   ├── Tile.cpp
│   ├── Grid.cpp
│   ├── Dictionary.cpp
│   ├── Player.cpp
│   └── GameEngine.cpp
├── assets/
│   ├── fonts/          # Place your .ttf font file here
│   └── words.txt       # Optional: one English word per line
├── CMakeLists.txt
└── README.md
```

---

## 🛠 Required Tools & Versions

| Tool | Minimum Version | Download |
|------|----------------|---------|
| **MinGW-w64** (g++) | 13.0+ | https://winlibs.com (winlibs-x86_64-posix-seh-gcc-14.3.0-mingw-w64ucrt-12.0.0-r1.7z)|
| **CMake** | 3.28+ | https://cmake.org/download (cmake-3.31.11-windows-x86_64.msi)| 
| **SFML** | 3.0.0 | *Auto-downloaded by CMake* |
| **VS Code** | Latest | https://code.visualstudio.com |
| **Git** | Any | https://git-scm.com (needed by CMake FetchContent) |

### VS Code Extensions Required
Install these from the Extensions panel (`Ctrl+Shift+X`):

| Extension | Publisher |
|-----------|-----------|
| C/C++ | Microsoft (ms-vscode.cpptools) |
| CMake Tools | Microsoft (ms-vscode.cmake-tools) |

---

## Installation & Setup

### Step 1 — Install MinGW-w64

1. Go to https://winlibs.com
2. Download the latest **GCC 13+** release for **Win64** (UCRT, without LLVM)
   - Filename example: `winlibs-x86_64-ucrt-win32-gcc-13.x.x-...-msvcrt.7z`
3. Extract the archive to `C:\mingw64`
4. Add `C:\mingw64\bin` to your Windows PATH:
   - Search → "Edit the system environment variables"
   - Click **Environment Variables** → select **Path** → **Edit**
   - Click **New** → type `C:\mingw64\bin` → OK all dialogs
5. Verify in a new terminal:
   ```
   g++ --version
   ```
   You should see `g++ (GCC) 13.x.x ...`

### Step 2 — Install CMake

1. Go to https://cmake.org/download
2. Download the **Windows x64 Installer** (`.msi`)
3. During installation, select **"Add CMake to the system PATH for all users"**
4. Verify:
   ```
   cmake --version
   ```
   You should see `cmake version 3.28.x` or higher.

### Step 3 — Install Git (if not already installed)

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

The game needs a TrueType font (`.ttf`). You have two options:

**Option A** — Use Windows' built-in Arial (easiest):
The game automatically looks for `C:\Windows\Fonts\arial.ttf`. No action needed.

**Option B** — Add a custom font:
1. Download Roboto from https://fonts.google.com/specimen/Roboto
2. Place `Roboto-Regular.ttf` inside `bookworm_project/assets/fonts/`

### Step 6 — (Optional) Add a Word List

A large dictionary makes the game much more fun.

1. Download `words_alpha.txt` from https://github.com/dwyl/english-words
2. Rename it to `words.txt`
3. Place it at `bookworm_project/assets/words.txt`

If this file is missing, the game falls back to a built-in list of ~1,000 common English words.

---

## How to Compile, Build, and Run

### Option A — VS Code (Recommended)

1. Open VS Code
2. **File → Open Folder** → select the `bookworm_project` folder
3. VS Code may prompt "Would you like to configure this project?" — click **Yes**
   - If not prompted, open the Command Palette (`Ctrl+Shift+P`) → **CMake: Configure**
4. Select your compiler kit when asked:
   - Choose **GCC 13.x.x (MinGW)** or similar
5. Build the project:
   ```
   Ctrl + Shift + B
   ```
   Or use Command Palette → **CMake: Build**

   > **First build takes 2–5 minutes** because CMake downloads and compiles SFML 3.0.0 from GitHub. Subsequent builds are fast.

6. Run the game:
   - Open the integrated terminal (`Ctrl + `` ` ``)
   ```
   .\build\Bookworm.exe
   ```
   Or use Command Palette → **CMake: Run Without Debugging**

---

### Option B — Terminal (Manual)

Open a terminal in the `bookworm_project` folder and run:

```bash
# Step 1: Configure (downloads SFML on first run)
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

# Step 2: Build
cmake --build build --parallel

# Step 3: Run
.\build\Bookworm.exe
```

For a Debug build (includes debug symbols for GDB):
```bash
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
```

---

### Option C — Debug with F5 (VS Code + GDB)

Create `.vscode/launch.json` in your project with:

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug Bookworm",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/Bookworm.exe",
            "cwd": "${workspaceFolder}/build",
            "MIMode": "gdb",
            "miDebuggerPath": "C:/mingw64/bin/gdb.exe",
            "preLaunchTask": "CMake Build"
        }
    ]
}
```

Then press **F5** to build and launch with the debugger attached.

---

## How to Play

A 4×4 grid of letter tiles is displayed. Spell valid English words by clicking adjacent tiles.

| Control | Action |
|---------|--------|
| **Left-click** a tile | Select it (must be adjacent to last selected) |
| **Left-click** the last tile | Deselect it (undo last letter) |
| **Enter** | Submit the current word |
| **Backspace** | Clear the entire selection |
| **Esc** | Pause / Resume |
| **R** | Restart the game |

### Rules
- Words must be **at least 3 letters** long.
- Each tile you select must be **8-directionally adjacent** (horizontal, vertical, or diagonal) to the previous tile.
- You cannot use the same tile twice in one word.
- The word must exist in the dictionary.

### Scoring
```
base  = sum of letter point values for each tile used
bonus = word length × 10   (words of 5 letters)
      = word length × 20   (words of 6–7 letters)
      = word length × 50   (words of 8+ letters)
total = (base + bonus) × current level
```

Letter values follow the Scrabble scale (A=1, Z=10, Q=10, etc.).

### Level Progression
Every **5 valid words** increases your level by 1. The level acts as a score multiplier, so longer words at higher levels yield dramatically more points.

---

## Architecture Overview

```
main()
  └── GameEngine::run()          ← owns everything
        │
        ├── processEvents()      ← polls SFML window events
        │     ├── onMousePressed()  → Grid::onMousePressed()
        │     └── onKeyPressed()    → trySubmitWord() / resetGame()
        │
        ├── update(dt)           ← advance timers, sync HUD labels
        │
        └── render()             ← draw all layers
              ├── Grid::draw()
              │     └── Tile::draw()   (×16)
              └── HUD text + overlays
```

### Class Responsibilities

| Class | Role |
|-------|------|
| **Tile** | One letter cell. Stores letter, grid position, and state (Normal / Selected / Burning). Draws itself as a coloured rectangle with a centred letter. |
| **Grid** | Owns all 16 `Tile` objects. Enforces adjacency rules during selection. On word submission, removes tiles, shifts columns down, and refills with new random letters. |
| **Dictionary** | Loads a word list into an `unordered_set` for O(1) lookups. Falls back to a 1,000-word built-in list if no file is found. |
| **Player** | Accumulates score, records word history, and handles level-up logic. Calculates word score with length bonuses and the level multiplier. |
| **GameEngine** | Top-level orchestrator. Owns all subsystems. Drives the SFML event loop with `processEvents → update → render` each frame. |

### Data Flow (word submission)
```
Player clicks tiles  →  Grid records selection (adjacency enforced)
Player presses ENTER →  GameEngine gets word from Grid
                     →  Dictionary::isValid(word)
                            YES → Player::addWord(word, tileSum)
                                  Grid::removeSelectedTiles()
                                  UI message "Nice word! +pts"
                            NO  → UI message "Not a word!"
                                  Grid::clearSelection()
```

---

## Common Issues

| Problem | Solution |
|---------|----------|
| `cmake` not found | Re-install CMake and make sure "Add to PATH" was ticked |
| `g++` not found | Check that `C:\mingw64\bin` is in your PATH (restart VS Code after adding) |
| Font not loading / blank window | Place `arial.ttf` next to `Bookworm.exe`, or add a `.ttf` to `assets/fonts/` |
| SFML download fails | Check your internet connection; Git must be installed |
| `ninja` error | Use `-G "MinGW Makefiles"` instead of letting CMake auto-detect the generator |

---

## Extension Ideas

- [ ] **Burning tiles** — tiles that spread fire if not cleared quickly
- [ ] **Power-up tiles** — wildcard letters, 2× score, bomb (clears a row)
- [ ] **Animations** — tile removal particle effects using SFML's `sf::CircleShape`
- [ ] **Sound effects** — add `SFML::Audio` for click, success, and fail sounds
- [ ] **High-score file** — persist the top 10 scores using `std::fstream`
- [ ] **Main menu screen** — state machine with Menu / Playing / GameOver states
- [ ] **Difficulty selector** — easy (5×5 board, common letters) vs hard (4×4, rare letters)
