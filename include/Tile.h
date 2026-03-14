#pragma once
#include <SFML/Graphics.hpp>
#include <optional>
#include <string>

// ============================================================
//  TileState  -  visual / gameplay state of one tile
// ============================================================
enum class TileState
{
    Normal,   // default tan colour
    Selected, // green  - tile is part of the current word
    Burning   // red    - tile will destroy the board if not cleared
};

// ============================================================
//  Tile  -  one letter cell on the 4x4 grid
//
//  NOTE: sf::Text has no default constructor in SFML 3, so
//  m_text is stored as std::optional<sf::Text> to allow
//  Tile to be default-constructed (needed by std::vector).
// ============================================================
class Tile
{
public:
    // -- Construction -----------------------------------------
    Tile() = default; // creates an "empty" uninitialised tile
    Tile(char letter, int col, int row,
         const sf::Font &font,
         sf::Vector2f pixelPos,
         float size);

    // -- Getters ----------------------------------------------
    char getLetter() const { return m_letter; }
    int getCol() const { return m_col; }
    int getRow() const { return m_row; }
    TileState getState() const { return m_state; }

    // -- Setters ----------------------------------------------
    void setState(TileState s);
    void setLetter(char c);

    // -- Interaction ------------------------------------------
    bool contains(sf::Vector2f point) const;

    // -- Rendering --------------------------------------------
    void draw(sf::RenderWindow &window, float totalTime) const;

    // -- Static utility ---------------------------------------
    static int letterValue(char c); // Scrabble-style point value

    // for burning tiles
    void tickBurn();       // Call this when a word is submitted
    void resetBurnTimer(); // Call this if the tile is cleared
    bool isBurning() const { return m_state == TileState::Burning; }
    int getBurnCounter() const { return m_burnCounter; }
    bool m_isPersistentBurn = false;
    bool isPersistentBurn() const { return m_isPersistentBurn; }
    void setGridPos(int col, int row);
    void setPosition(sf::Vector2f pixelPos);
    void centerText();

private:
    char m_letter = 'A';
    int m_col = 0;
    int m_row = 0;
    TileState m_state = TileState::Normal;

    sf::RectangleShape m_background;
    std::optional<sf::Text> m_text; // optional: sf::Text has no default ctor
    std::optional<sf::Text> m_burnText;

    //   for burning tiles
    int m_burnCounter = 0;
    static constexpr int MAX_BURN_STEPS = 3; // How many turns until it "explodes"

    void updateColors();
};