#include "Grid.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>

// -- Weighted letter pools for better word formation ---------------------------
// Separating pools allows us to enforce a ~42% vowel ratio, ensuring playability.

// Vowels: A(9), E(12), I(9), O(8), U(4)
static const char VOWEL_POOL[] = 
    "AAAAAAAAAEEEEEEEEEEEEIIIIIIIIIOOOOOOOOUUUU";
static const int VOWEL_SIZE = static_cast<int>(sizeof(VOWEL_POOL) - 1);

// Consonants: Boosted S, T, R, N, L for better connectivity.
static const char CONSONANT_POOL[] =
    "BBCCDDDDFFGGGHHJKLLLLLMMNNNNNNPPQRRRRRRSSSSSSTTTTTTVVWWXYYZ";
static const int CONSONANT_SIZE = static_cast<int>(sizeof(CONSONANT_POOL) - 1);

// ------------------------------------------------------------------------------
//  Constructor  -  seeds RNG and fills the board with random letters
// ------------------------------------------------------------------------------
Grid::Grid(const sf::Font &font, sf::Vector2f origin)
    : m_font(&font), m_origin(origin)
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // Allocate COLS empty columns
    m_tiles.resize(Grid::COLS);

    // Fill each column with ROWS fully-constructed Tile objects.
    // We cannot use std::vector<Tile>(ROWS) because Tile's default
    // constructor leaves sf::Text uninitialised (SFML 3 has no
    // default-constructible sf::Text). We use reserve + emplace_back instead.
    for (int c = 0; c < Grid::COLS; ++c)
    {
        m_tiles[c].reserve(Grid::ROWS);
        for (int r = 0; r < Grid::ROWS; ++r)
            m_tiles[c].emplace_back(randomLetter(), c, r,
                                    *m_font, tilePixelPos(c, r), Grid::TILE_SIZE);
    }
}

// ------------------------------------------------------------------------------
//  onMousePressed
//  Rules:
//    - Clicking the last selected tile deselects it (backspace behaviour)
//    - New tiles must be 8-directionally adjacent to the last selected tile
//    - A tile already in the selection chain cannot be added again
// ------------------------------------------------------------------------------
void Grid::onMousePressed(sf::Vector2f pos)
{
    for (int c = 0; c < Grid::COLS; ++c)
    {
        for (int r = 0; r < Grid::ROWS; ++r)
        {
            Tile &t = m_tiles[c][r];
            if (!t.contains(pos))
                continue;

            // Deselect last tile if clicked again
            if (!m_selected.empty() && m_selected.back() == &t)
            {
                t.setState(TileState::Normal);
                m_selected.pop_back();
                return;
            }
            // Adjacency check (skip for the very first tile)
            if (!m_selected.empty() && !isAdjacent(m_selected.back(), &t))
                return;
            // No duplicates
            if (isAlreadySelected(&t))
                return;

            t.setState(TileState::Selected);
            m_selected.push_back(&t);
            return;
        }
    }
}

// ------------------------------------------------------------------------------
//  Word helpers
// ------------------------------------------------------------------------------
std::string Grid::getSelectedWord() const
{
    std::string word;
    for (const Tile *t : m_selected)
        word += t->getLetter();
    return word;
}

void Grid::clearSelection()
{
    for (Tile *t : m_selected)
        if (t->getState() == TileState::Selected)
            t->setState(TileState::Normal);
    m_selected.clear();
}

void Grid::removeSelectedTiles(int currentLevel)
{
    // 1. Mark tiles for removal
    for (Tile *t : m_selected)
        t->setLetter('\0');

    // 2. Clear selection immediately to prevent dangling pointers
    m_selected.clear();

    // 3. Shift affected columns (Using the move logic we built)
    for (int c = 0; c < Grid::COLS; ++c) {
        bool columnHasSpace = false;
        for (int r = 0; r < Grid::ROWS; ++r) {
            if (m_tiles[c][r].getLetter() == '\0') columnHasSpace = true;
        }
        if (columnHasSpace) fillColumn(c, currentLevel);
    }

    // 4. Tick Burn Counters ONLY for tiles that are still on the board
    // Doing this AFTER fillColumn prevents ticking tiles that were just deleted
    for (int c = 0; c < Grid::COLS; ++c) {
        for (int r = 0; r < Grid::ROWS; ++r) {
            m_tiles[c][r].tickBurn();
        }
    }

    // 5. Random Ignition Logic
    if ((std::rand() % 100) < 20) {
        int rC = std::rand() % Grid::COLS;
        int rR = std::rand() % Grid::ROWS;
        // Ensure we don't ignite a tile that just spawned or is already burning
        if (m_tiles[rC][rR].getState() == TileState::Normal) {
            m_tiles[rC][rR].setState(TileState::Burning);
        }
    }
}

// ------------------------------------------------------------------------------
//  Rendering
// ------------------------------------------------------------------------------
void Grid::draw(sf::RenderWindow &window, float totalTime) const
{
    // Wooden board background
    float boardW = Grid::COLS * (Grid::TILE_SIZE + Grid::TILE_GAP) + Grid::TILE_GAP;
    float boardH = Grid::ROWS * (Grid::TILE_SIZE + Grid::TILE_GAP) + Grid::TILE_GAP;
    sf::RectangleShape bg({boardW, boardH});
    bg.setPosition(m_origin - sf::Vector2f(Grid::TILE_GAP, Grid::TILE_GAP));
    bg.setFillColor(sf::Color(90, 60, 30));
    bg.setOutlineThickness(4.f);
    bg.setOutlineColor(sf::Color(50, 30, 10));
    window.draw(bg);

    for (int c = 0; c < Grid::COLS; ++c)
        for (int r = 0; r < Grid::ROWS; ++r)
            m_tiles[c][r].draw(window, totalTime);
}

// ------------------------------------------------------------------------------
//  Private helpers
// ------------------------------------------------------------------------------
char Grid::randomLetter()
{
    // 42% chance for a vowel (slightly higher than English ~38% to aid flow).
    // This algorithm prevents "impossible" boards that run out of vowels.
    if ((std::rand() % 100) < 42) {
        return VOWEL_POOL[std::rand() % VOWEL_SIZE];
    } else {
        return CONSONANT_POOL[std::rand() % CONSONANT_SIZE];
    }
}

sf::Vector2f Grid::tilePixelPos(int col, int row) const
{
    return m_origin + sf::Vector2f(
                          static_cast<float>(col) * (Grid::TILE_SIZE + Grid::TILE_GAP),
                          static_cast<float>(row) * (Grid::TILE_SIZE + Grid::TILE_GAP));
}

void Grid::fillColumn(int col, int currentLevel)
{
    // 1. Collect the actual Tile objects that survived (not marked '\0')
    std::vector<Tile> survivors;
    for (int r = 0; r < Grid::ROWS; ++r)
    {
        if (m_tiles[col][r].getLetter() != '\0')
        {
            // Use std::move to transfer the state (burning, counter, etc.)
            survivors.push_back(std::move(m_tiles[col][r]));
        }
    }

    int numNewTiles = Grid::ROWS - static_cast<int>(survivors.size());

    // 2. Clear the column so we can rebuild it from top to bottom
    m_tiles[col].clear();
    m_tiles[col].reserve(Grid::ROWS);

    // 3. Add the BRAND NEW tiles at the top
    for (int r = 0; r < numNewTiles; ++r)
    {
        m_tiles[col].emplace_back(randomLetter(), col, r, *m_font, tilePixelPos(col, r), Grid::TILE_SIZE);
        
        // Logic for spawning new hazards
        if (currentLevel > 1)
        {
            // Using your testing probability (very high!)
            if ((std::rand() % 100) < (30 + (currentLevel * 30)))
            {
                m_tiles[col].back().setState(TileState::Burning);
            }
        }
    }

    // 4. Put the SURVIVORS back into the column, underneath the new tiles
    for (int i = 0; i < (int)survivors.size(); ++i)
    {
        int newRow = numNewTiles + i; // Positioned below the newly spawned tiles
        
        m_tiles[col].push_back(std::move(survivors[i]));

        // CRITICAL: Update the survivor's internal logic and visual position
        // This ensures the burning red color and text move with the tile
        m_tiles[col].back().setGridPos(col, newRow);
        m_tiles[col].back().setPosition(tilePixelPos(col, newRow));
    }
}

void Grid::spawnBurningTile(int count) {
    for (int i = 0; i < count; ++i) {
        int c = std::rand() % Grid::COLS;
        int r = std::rand() % Grid::ROWS;
        
        // Don't burn a tile that is already burning or selected
        if (m_tiles[c][r].getState() == TileState::Normal) {
            m_tiles[c][r].setState(TileState::Burning);
        }
    }
}

bool Grid::isAlreadySelected(const Tile *t) const
{
    return std::find(m_selected.begin(), m_selected.end(), t) != m_selected.end();
}

bool Grid::isAdjacent(const Tile *a, const Tile *b) const
{
    int dc = std::abs(a->getCol() - b->getCol());
    int dr = std::abs(a->getRow() - b->getRow());
    return (dc <= 1 && dr <= 1) && (dc + dr > 0); // 8-directional, not self
}

// for burning tiles
bool Grid::hasExploded() const {
    for (int c = 0; c < Grid::COLS; ++c) {
        for (int r = 0; r < Grid::ROWS; ++r) {
            if (m_tiles[c][r].isBurning() && m_tiles[c][r].getBurnCounter() <= 0) {
                return true;
            }
        }
    }
    return false;
}
