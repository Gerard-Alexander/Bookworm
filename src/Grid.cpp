#include "Grid.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>

// -- Weighted letter pool (common letters appear more often) -------------------
static const char LETTER_POOL[] =
    "AAAAAAAAABBBCCCDDDDEEEEEEEEEEFFFGGGHHHIIIIIIIIIJKLL"
    "LLMMMNNNNNNOOOOOOOOOPPPQRRRRRRSSSSTTTTTTTUUUUVVWWXYYZ";
static const int POOL_SIZE = static_cast<int>(sizeof(LETTER_POOL) - 1);

// ------------------------------------------------------------------------------
//  Constructor  -  seeds RNG and fills the board with random letters
// ------------------------------------------------------------------------------
Grid::Grid(const sf::Font& font, sf::Vector2f origin)
    : m_font(&font), m_origin(origin)
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // Allocate COLS empty columns
    m_tiles.resize(COLS);

    // Fill each column with ROWS fully-constructed Tile objects.
    // We cannot use std::vector<Tile>(ROWS) because Tile's default
    // constructor leaves sf::Text uninitialised (SFML 3 has no
    // default-constructible sf::Text). We use reserve + emplace_back instead.
    for (int c = 0; c < COLS; ++c) {
        m_tiles[c].reserve(ROWS);
        for (int r = 0; r < ROWS; ++r)
            m_tiles[c].emplace_back(randomLetter(), c, r,
                                    *m_font, tilePixelPos(c, r), TILE_SIZE);
    }
}

// ------------------------------------------------------------------------------
//  onMousePressed
//  Rules:
//    - Clicking the last selected tile deselects it (backspace behaviour)
//    - New tiles must be 8-directionally adjacent to the last selected tile
//    - A tile already in the selection chain cannot be added again
// ------------------------------------------------------------------------------
void Grid::onMousePressed(sf::Vector2f pos) {
    for (int c = 0; c < COLS; ++c) {
        for (int r = 0; r < ROWS; ++r) {
            Tile& t = m_tiles[c][r];
            if (!t.contains(pos)) continue;

            // Deselect last tile if clicked again
            if (!m_selected.empty() && m_selected.back() == &t) {
                t.setState(TileState::Normal);
                m_selected.pop_back();
                return;
            }
            // Adjacency check (skip for the very first tile)
            if (!m_selected.empty() && !isAdjacent(m_selected.back(), &t))
                return;
            // No duplicates
            if (isAlreadySelected(&t)) return;

            t.setState(TileState::Selected);
            m_selected.push_back(&t);
            return;
        }
    }
}

// ------------------------------------------------------------------------------
//  Word helpers
// ------------------------------------------------------------------------------
std::string Grid::getSelectedWord() const {
    std::string word;
    for (const Tile* t : m_selected)
        word += t->getLetter();
    return word;
}

void Grid::clearSelection() {
    for (Tile* t : m_selected)
        if (t->getState() == TileState::Selected)
            t->setState(TileState::Normal);
    m_selected.clear();
}

void Grid::removeSelectedTiles() {
    // Track which columns are affected
    std::vector<bool> affectedCols(COLS, false);
    for (const Tile* t : m_selected)
        affectedCols[t->getCol()] = true;

    // Mark removed positions with a sentinel letter '\0'
    for (Tile* t : m_selected)
        t->setLetter('\0');

    m_selected.clear();

    // Shift each affected column down and refill the top
    for (int c = 0; c < COLS; ++c)
        if (affectedCols[c]) fillColumn(c);
}

// ------------------------------------------------------------------------------
//  Rendering
// ------------------------------------------------------------------------------
void Grid::draw(sf::RenderWindow& window) const {
    // Wooden board background
    float boardW = COLS * (TILE_SIZE + TILE_GAP) + TILE_GAP;
    float boardH = ROWS * (TILE_SIZE + TILE_GAP) + TILE_GAP;
    sf::RectangleShape bg({boardW, boardH});
    bg.setPosition(m_origin - sf::Vector2f(TILE_GAP, TILE_GAP));
    bg.setFillColor(sf::Color(90, 60, 30));
    bg.setOutlineThickness(4.f);
    bg.setOutlineColor(sf::Color(50, 30, 10));
    window.draw(bg);

    for (int c = 0; c < COLS; ++c)
        for (int r = 0; r < ROWS; ++r)
            m_tiles[c][r].draw(window);
}

// ------------------------------------------------------------------------------
//  Private helpers
// ------------------------------------------------------------------------------
char Grid::randomLetter() {
    return LETTER_POOL[std::rand() % POOL_SIZE];
}

sf::Vector2f Grid::tilePixelPos(int col, int row) const {
    return m_origin + sf::Vector2f(
        static_cast<float>(col) * (TILE_SIZE + TILE_GAP),
        static_cast<float>(row) * (TILE_SIZE + TILE_GAP)
    );
}

void Grid::fillColumn(int col) {
    // Collect surviving letters (skip sentinel '\0')
    std::vector<char> letters;
    for (int r = 0; r < ROWS; ++r) {
        char ch = m_tiles[col][r].getLetter();
        if (ch != '\0') letters.push_back(ch);
    }
    // Prepend fresh random letters for the removed slots
    while (static_cast<int>(letters.size()) < ROWS)
        letters.insert(letters.begin(), randomLetter());

    // Rebuild each tile in the column using emplace (no default ctor needed)
    m_tiles[col].clear();
    m_tiles[col].reserve(ROWS);
    for (int r = 0; r < ROWS; ++r)
        m_tiles[col].emplace_back(letters[r], col, r,
                                   *m_font, tilePixelPos(col, r), TILE_SIZE);
}

bool Grid::isAlreadySelected(const Tile* t) const {
    return std::find(m_selected.begin(), m_selected.end(), t) != m_selected.end();
}

bool Grid::isAdjacent(const Tile* a, const Tile* b) const {
    int dc = std::abs(a->getCol() - b->getCol());
    int dr = std::abs(a->getRow() - b->getRow());
    return (dc <= 1 && dr <= 1) && (dc + dr > 0);   // 8-directional, not self
}