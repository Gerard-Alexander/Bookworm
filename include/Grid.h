#pragma once
#include "Tile.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

// ============================================================
//  Grid  –  the 4×4 board of Tile objects
//
//  Knows:
//    • All 16 Tiles stored in a 2-D vector [col][row]
//    • Which tiles the player has selected (and in what order)
//    • Board pixel origin and tile sizing
//
//  Can:
//    • Handle mouse-click events and apply adjacency rules
//    • Build the current word from selected tiles
//    • Remove accepted-word tiles, shift columns down,
//      and refill empty cells with new random letters
//    • Draw the board background and every tile
// ============================================================
class Grid
{
public:
    // ── Board constants ──────────────────────────────────────
    static constexpr int COLS = 8;
    static constexpr int ROWS = 8;
    static constexpr float TILE_SIZE = 60.f;
    static constexpr float TILE_GAP = 4.f;

    // ── Construction ─────────────────────────────────────────
    Grid(const sf::Font &font, sf::Vector2f origin);

    // ── Mouse events ─────────────────────────────────────────
    void onMousePressed(sf::Vector2f pos);

    // ── Word helpers ─────────────────────────────────────────
    std::string getSelectedWord() const;
    void clearSelection();
    void removeSelectedTiles(int currentLevel); // call after a valid word

    // ── Accessor ─────────────────────────────────────────────
    const std::vector<Tile *> &getSelectedTiles() const { return m_selected; }

    // ── Rendering ────────────────────────────────────────────
    void draw(sf::RenderWindow &window, float totalTime) const;

    // for spawning burning tiles
    void fillColumn(int col, int currentLevel); // Added level parameter
    void spawnBurningTile(int count = 1);       // Force-turns random tiles to Burning
    bool hasExploded() const;

private:
    std::vector<std::vector<Tile>> m_tiles; // [col][row]
    std::vector<Tile *> m_selected;         // selection order matters

    const sf::Font *m_font = nullptr;
    sf::Vector2f m_origin;

    // ── Private helpers ──────────────────────────────────────
    static char randomLetter();
    sf::Vector2f tilePixelPos(int col, int row) const;
    void fillColumn(int col);
    bool isAlreadySelected(const Tile *t) const;
    bool isAdjacent(const Tile *a, const Tile *b) const;
    
};
