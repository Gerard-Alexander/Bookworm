#include "Grid.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>

static const char VOWEL_POOL[] =
    "AAAAAAAAAEEEEEEEEEEEEIIIIIIIIIOOOOOOOOUUUU";
static const int VOWEL_SIZE = static_cast<int>(sizeof(VOWEL_POOL) - 1);

static const char CONSONANT_POOL[] =
    "BBCCDDDDFFGGGHHJKLLLLLMMNNNNNNPPQRRRRRRSSSSSSTTTTTTVVWWXYYZ";
static const int CONSONANT_SIZE = static_cast<int>(sizeof(CONSONANT_POOL) - 1);

// ------------------------------------------------------------------------------
//  Constructor
// ------------------------------------------------------------------------------
Grid::Grid(const sf::Font &font, sf::Vector2f origin)
    : m_font(&font), m_origin(origin)
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    m_tiles.resize(Grid::COLS);
    for (int c = 0; c < m_tiles.size(); ++c)
    {
        m_tiles[c].reserve(Grid::ROWS);
        for (int r = 0; r < Grid::ROWS; ++r)
            m_tiles[c].emplace_back(randomLetter(), c, r,
                                    *m_font, tilePixelPos(c, r), Grid::TILE_SIZE);
    }
}

// ------------------------------------------------------------------------------
//  onMousePressed
// ------------------------------------------------------------------------------
void Grid::onMousePressed(sf::Vector2f pos)
{
    for (int c = 0; c < m_tiles.size(); ++c)
    {
        for (int r = 0; r < m_tiles[c].size(); ++r)
        {
            Tile &t = m_tiles[c][r];
            if (!t.contains(pos)) continue;

            if (!m_selected.empty() && m_selected.back() == &t)
            {
                t.setState(TileState::Normal);
                m_selected.pop_back();
                return;
            }
            if (!m_selected.empty() && !isAdjacent(m_selected.back(), &t))
                return;
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

    // 3. Refill affected columns
    for (int c = 0; c < (int)m_tiles.size(); ++c) {
        bool columnHasSpace = false;
        for (int r = 0; r < (int)m_tiles[c].size(); ++r)
            if (m_tiles[c][r].getLetter() == '\0') columnHasSpace = true;
        if (columnHasSpace) fillColumn(c, currentLevel);
    }

    // 4. Tick burn counters ONCE here for the whole board.
    //    Skip tiles that just spawned at MAX_BURN_STEPS (brand new burning tiles).
    for (int c = 0; c <(int)m_tiles.size(); ++c) {
        for (int r = 0; r < (int)m_tiles[c].size(); ++r) {
            if (m_tiles[c][r].isBurning()) {
                m_tiles[c][r].tickBurn();
            }
        }
    }

    // 5. Random ignition - only from level 3+, max 2 burning tiles
    int burningCount = 0;
    for (int c = 0; c < (int)m_tiles.size(); ++c)
        for (int r = 0; r < (int)m_tiles[c].size(); ++r)
            if (m_tiles[c][r].isBurning()) ++burningCount;

    if (burningCount < 2 && currentLevel > 2 && (std::rand() % 100) < 5) {
        int rC = std::rand() % Grid::COLS;
        int rR = std::rand() % Grid::ROWS;
        if (m_tiles[rC][rR].getState() == TileState::Normal)
            m_tiles[rC][rR].setState(TileState::Burning);
    }
}

// ------------------------------------------------------------------------------
//  Rendering
// ------------------------------------------------------------------------------
void Grid::draw(sf::RenderWindow &window, float totalTime) const
{
    float boardW = Grid::COLS * (Grid::TILE_SIZE + Grid::TILE_GAP) + Grid::TILE_GAP;
    float boardH = Grid::ROWS * (Grid::TILE_SIZE + Grid::TILE_GAP) + Grid::TILE_GAP;
    sf::RectangleShape bg({boardW, boardH});
    bg.setPosition(m_origin - sf::Vector2f(Grid::TILE_GAP, Grid::TILE_GAP));
    bg.setFillColor(sf::Color(90, 60, 30));
    bg.setOutlineThickness(4.f);
    bg.setOutlineColor(sf::Color(50, 30, 10));
    window.draw(bg);

    for (int c = 0; c < (int)m_tiles.size(); ++c)
        for (int r = 0; r < (int)m_tiles[c].size(); ++r)
            m_tiles[c][r].draw(window, totalTime);
}

// ------------------------------------------------------------------------------
//  Private helpers
// ------------------------------------------------------------------------------
char Grid::randomLetter()
{
    if ((std::rand() % 100) < 42)
        return VOWEL_POOL[std::rand() % VOWEL_SIZE];
    else
        return CONSONANT_POOL[std::rand() % CONSONANT_SIZE];
}

sf::Vector2f Grid::tilePixelPos(int col, int row) const
{
    return m_origin + sf::Vector2f(
        static_cast<float>(col) * (Grid::TILE_SIZE + Grid::TILE_GAP),
        static_cast<float>(row) * (Grid::TILE_SIZE + Grid::TILE_GAP));
}

// ------------------------------------------------------------------------------
//  fillColumn
//  Rebuilds one column after tiles have been removed.
//  NOTE: tickBurn is NOT called here - it is called once in removeSelectedTiles.
// ------------------------------------------------------------------------------
void Grid::fillColumn(int col, int currentLevel)
{
    // 1. Collect surviving tiles (not marked '\0')
    std::vector<Tile> survivors;
    for (int r = 0; r < (int)m_tiles[col].size(); ++r)
        if (m_tiles[col][r].getLetter() != '\0')
            survivors.push_back(std::move(m_tiles[col][r]));

    int numNewTiles = Grid::ROWS - static_cast<int>(survivors.size());

    // 2. Clear and rebuild the column
    m_tiles[col].clear();
    m_tiles[col].reserve(Grid::ROWS);

    // 3. Add new tiles at the top
    for (int r = 0; r < numNewTiles; ++r)
    {
        m_tiles[col].emplace_back(randomLetter(), col, r,
                                   *m_font, tilePixelPos(col, r), Grid::TILE_SIZE);

        // Burn spawn: level 4+, rows 0-5 only, max 2 burning tiles total
        if (currentLevel > 3 && r <= Grid::ROWS - 3)
        {
            int burningCount = 0;
            for (int c = 0; c < m_tiles.size(); ++c)
                for (int rr = 0; rr < m_tiles[c].size(); ++rr)
                    if (m_tiles[c][rr].isBurning()) ++burningCount;

            if (burningCount < 2 && (std::rand() % 100) < 8)
            {
                // Require at least one vowel neighbour
                bool hasVowelNeighbour = false;
                const std::string vowels = "AEIOU";
                for (int dc = -1; dc <= 1 && !hasVowelNeighbour; ++dc)
                    for (int dr = -1; dr <= 1 && !hasVowelNeighbour; ++dr) {
                        if (dc == 0 && dr == 0) continue;
                        int nc = col + dc, nr = r + dr;
                        if (nc < 0 || nc >= Grid::COLS) continue;
                        if (nr < 0 || nr >= Grid::ROWS) continue;
                        if (nr >= (int)m_tiles[nc].size()) continue;
                        char nb = m_tiles[nc][nr].getLetter();
                        if (vowels.find(nb) != std::string::npos)
                            hasVowelNeighbour = true;
                    }
                if (hasVowelNeighbour)
                    m_tiles[col].back().setState(TileState::Burning);
            }
        }
    }

    // 4. Put survivors back underneath the new tiles
    for (int i = 0; i < (int)survivors.size(); ++i)
    {
        int newRow = numNewTiles + i;
        m_tiles[col].push_back(std::move(survivors[i]));
        m_tiles[col].back().setGridPos(col, newRow);
        m_tiles[col].back().setPosition(tilePixelPos(col, newRow));
    }
    while ((int)m_tiles[col].size() < Grid::ROWS) {
        int r = m_tiles[col].size();
        m_tiles[col].emplace_back(randomLetter(), col, r,
                                  *m_font, tilePixelPos(col, r), Grid::TILE_SIZE);
    }
    
}

// ------------------------------------------------------------------------------
//  spawnBurningTile
// ------------------------------------------------------------------------------
void Grid::spawnBurningTile(int count) {
    for (int i = 0; i < count; ++i) {
        // Hard cap: never more than 2 burning tiles
        int burningCount = 0;
        for (int c = 0; c < (int)m_tiles.size(); ++c)
            for (int r = 0; r < (int)m_tiles[c].size(); ++r)
                if (m_tiles[c][r].isBurning()) ++burningCount;
        if (burningCount >= 2) return;

        for (int attempt = 0; attempt < 20; ++attempt) {
            int c = std::rand() % Grid::COLS;
            int r = std::rand() % Grid::ROWS;   // pick a row index

            if (r >= (int)m_tiles[c].size()) continue;

            if (m_tiles[c][r].getState() != TileState::Normal) continue;

            // Require at least one vowel neighbour
            bool hasVowelNeighbour = false;
            const std::string vowels = "AEIOU";
            for (int dc = -1; dc <= 1 && !hasVowelNeighbour; ++dc)
                for (int dr = -1; dr <= 1 && !hasVowelNeighbour; ++dr) {
                    if (dc == 0 && dr == 0) continue;
                    int nc = c + dc, nr = r + dr;
                    if (nc < 0 || nc >= Grid::COLS) continue;
                    if (nr < 0 || nr >= (int)m_tiles[nc].size()) continue;
                    char neighbour = m_tiles[nc][nr].getLetter();
                    if (vowels.find(neighbour) != std::string::npos)
                        hasVowelNeighbour = true;
                }

            if (hasVowelNeighbour) {
                m_tiles[c][r].setState(TileState::Burning);
                break;
            }
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
    return (dc <= 1 && dr <= 1) && (dc + dr > 0);
}

bool Grid::hasExploded() const {
    for (int c = 0; c < (int)m_tiles.size(); ++c) {
        for (int r = 0; r < (int)m_tiles[c].size(); ++r) {
            if (m_tiles[c][r].isBurning() &&
                m_tiles[c][r].getBurnCounter() <= 0) {
                return true;
            }
        }
    }
    return false;
}
bool Grid::hasPossibleWord(const Dictionary& dict) const {
    std::vector<std::vector<bool>> visited(
        Grid::COLS, std::vector<bool>(Grid::ROWS, false));
    bool found = false;
    for (int c = 0; c < Grid::COLS && !found; ++c)
        for (int r = 0; r <(int)m_tiles[c].size() && !found; ++r) {
            std::string current;
            dfsWord(c, r, current, visited, dict, found);
        }
    return found;
}

void Grid::dfsWord(int col, int row,
                   std::string& current,
                   std::vector<std::vector<bool>>& visited,
                   const Dictionary& dict,
                   bool& found) const
{
    if (found) return;
    if (col < 0 || col >= Grid::COLS) return;
    if (row < 0 || row >= Grid::ROWS) return;
    if (visited[col][row]) return;
    if (static_cast<int>(current.size()) >= 8) return;

    visited[col][row] = true;
    current += m_tiles[col][row].getLetter();

    if (static_cast<int>(current.size()) >= Dictionary::MIN_WORD_LENGTH
        && dict.isValid(current))
    {
        found = true;
        current.pop_back();
        visited[col][row] = false;
        return;
    }

    for (int dc = -1; dc <= 1 && !found; ++dc)
        for (int dr = -1; dr <= 1 && !found; ++dr) {
            if (dc == 0 && dr == 0) continue;
            dfsWord(col + dc, row + dr, current, visited, dict, found);
        }

    current.pop_back();
    visited[col][row] = false;
}
int Grid::getBurnCount() const {
    int count = 0;
    for (int c = 0; c < (int)m_tiles.size(); ++c) {
        for (int r = 0; r < (int)m_tiles[c].size(); ++r) {
            if (m_tiles[c][r].isBurning()) {
                ++count;
            }
        }
    }
    return count;
}
void Grid::clearExplodedTiles() {
    for (int c = 0; c < (int)m_tiles.size(); ++c) {
        for (int r = 0; r < (int)m_tiles[c].size(); ++r) {
            if (m_tiles[c][r].isBurning() &&
                m_tiles[c][r].getBurnCounter() <= 0) {
                // Replace with a fresh normal tile
                m_tiles[c][r].setLetter(randomLetter());
                m_tiles[c][r].setState(TileState::Normal);
                m_tiles[c][r].resetBurnCounter();
            }
        }
    }
}

bool Grid::burnTileReachedBottom() const {
    for (int c = 0; c < (int)m_tiles.size(); ++c) {
        for (int r = 0; r < (int)m_tiles[c].size(); ++r) {
            if (m_tiles[c][r].isBurning() &&
                m_tiles[c][r].getRow() == Grid::ROWS - 1) {
                return true;
            }
        }
    }
    return false;
}

