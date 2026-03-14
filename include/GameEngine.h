#pragma once
#include "Grid.h"
#include "Dictionary.h"
#include "Player.h"
#include <SFML/Graphics.hpp>
#include <string>

// ============================================================
//  GameState  –  top-level states of the game
// ============================================================
enum class GameState {
    Playing,    // normal gameplay
    Paused,     // ESC pressed
    GameOver    // burn tile reached the top (future feature hook)
};

// ============================================================
//  GameEngine  –  top-level orchestrator
//
//  Owns every subsystem and wires them together:
//
//    ┌─────────────┐  mouse click   ┌──────┐
//    │ GameEngine  │──────────────▶ │ Grid │
//    │             │ ◀─selectedWord─│      │
//    │             │                └──────┘
//    │             │  isValid(word) ┌────────────┐
//    │             │──────────────▶ │ Dictionary │
//    │             │                └────────────┘
//    │             │  addWord(...)  ┌────────┐
//    │             │──────────────▶ │ Player │
//    │             │                └────────┘
//    └─────────────┘
//
//  Main game loop (called from run()):
//    1. processEvents()  – poll SFML events, dispatch to handlers
//    2. update(dt)       – advance timers, sync HUD text
//    3. render()         – clear → draw Grid, HUD, overlays → display
// ============================================================
class GameEngine {
public:
    GameEngine();

    // Entry point – opens the window, blocks until it is closed
    void run();

private:
    // ── SFML ─────────────────────────────────────────────────
    sf::RenderWindow m_window;
    sf::Font         m_font;

    // ── Subsystems ───────────────────────────────────────────
    Grid       m_grid;
    Dictionary m_dictionary;
    Player     m_player;

    // ── HUD text objects ─────────────────────────────────────
    sf::Text m_scoreText;
    sf::Text m_levelText;
    sf::Text m_wordText;
    sf::Text m_messageText;
    sf::Text m_hintText;
    sf::Text m_titleText;

    // ── State ────────────────────────────────────────────────
    GameState m_state        = GameState::Playing;
    float     m_messageTimer = 0.f;   // seconds left to show feedback
    float     m_totalTime    = 0.f;   // for animations (burning pulse)

    // ── Game-loop phases ─────────────────────────────────────
    void processEvents();
    void update(float dt);
    void render();

    // ── Event handlers ───────────────────────────────────────
    void onMousePressed(sf::Vector2f pos);
    void onKeyPressed(sf::Keyboard::Key key);

    // ── Word submission ──────────────────────────────────────
    void trySubmitWord();

    // ── Helpers ──────────────────────────────────────────────
    void  resetGame();
    bool  loadFont();
    void  initHUDText();
    void  showMessage(const std::string& msg, float duration = 1.5f);
    void  drawOverlay(const std::string& title,
                      const std::string& subtitle);

    // ── Layout constants ─────────────────────────────────────
    static constexpr unsigned WIN_W   = 600u;
    static constexpr unsigned WIN_H   = 800u;
    static constexpr float    BOARD_X = 42.f;
    static constexpr float    BOARD_Y = 140.f;
};
