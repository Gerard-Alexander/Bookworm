#pragma once
#include "Grid.h"
#include "Dictionary.h"
#include "Player.h"
#include <SFML/Graphics.hpp>
#include <optional>
#include <string>
#include <vector>
#include <SFML/Audio.hpp>
// ============================================================
//  GameState
// ============================================================
enum class GameState {
    Menu,
    Playing,
    Paused,
    GameOver,
    Help 
};

// ============================================================
//  Button  -  reusable clickable UI element
//
//  SFML 3 has no default constructor for sf::Text, so
//  label is stored as std::optional<sf::Text> — same fix
//  we used for Tile.
// ============================================================
struct Button {
    sf::RectangleShape       shape;  
    std::optional<sf::Text>  label;   // optional avoids default-ctor problem
    bool                     hovered = false;

    void init(const sf::Font& font,
              const std::string& text,
              sf::Vector2f pos,
              sf::Vector2f size,
              unsigned fontSize = 16u);

    bool contains(sf::Vector2f point) const;
    void setHover(bool h);
    void draw(sf::RenderWindow& window) const;
};

// ============================================================
//  GameEngine
// ============================================================
class GameEngine {
public:
    GameEngine();
    void run();

private:
    // ── SFML ─────────────────────────────────────────────────
    sf::RenderWindow m_window;
    sf::Font         m_font;

    // ── Subsystems ───────────────────────────────────────────
    Grid       m_grid;
    Dictionary m_dictionary;
    Player     m_player;

    // ── HUD text (original) ──────────────────────────────────
    sf::Text m_scoreText;
    sf::Text m_levelText;
    sf::Text m_wordText;
    sf::Text m_messageText;
    sf::Text m_hintText;
    sf::Text m_titleText;

    sf::Music m_backgroundMusic;
    sf::SoundBuffer m_clickBuffer;
    std::optional<sf::Sound> m_clickSound;
    sf::SoundBuffer m_damageBuffer;
    std::optional<sf::Sound> m_damageSound;
    // ── HUD text (Member B added) ─────────────────────────────
    sf::Text m_warningText;

    // ── State (original) ─────────────────────────────────────
    GameState m_state        = GameState::Menu;
    float     m_messageTimer = 0.f;
    float     m_totalTime    = 0.f;
    int m_highScore = 0;
    // ── State (Member B added) ────────────────────────────────
    bool m_noPossibleWord = false;
    void saveHighScore();
    // ── Toolbar buttons ──────────────────────────────────────
    Button m_btnSubmit;
    Button m_btnClear;
    Button m_btnPause;
    Button m_btnNewGame;
    Button m_btnHelp;
    Button m_btnExit;

    // ── Menu buttons ─────────────────────────────────────────
    Button m_btnPlay;
    Button m_btnMenuExit;

    // ── Game-loop phases ─────────────────────────────────────
    void processEvents();
    void update(float dt);
    void render();

    // ── Renderers ────────────────────────────────────────────
    void renderMenu();
    void renderGame();
    void renderPauseOverlay();
    void renderGameOverOverlay();

    // ── Sidebar (Member C fills in body) ─────────────────────
    void drawSidebar();

    // ── Event handlers ───────────────────────────────────────
    void onMousePressed(sf::Vector2f pos);
    void onMouseMoved(sf::Vector2f pos);
    void onKeyPressed(sf::Keyboard::Key key);
    void playClickSound();
    void renderHelp();
    // ── Word submission ──────────────────────────────────────
    void trySubmitWord();
    
    // ── Helpers (original names kept) ────────────────────────
    void  resetGame();
    bool  loadFont();
    void  initHUDText();
    void  showMessage(const std::string& msg, float duration = 1.5f);
    void  drawOverlay(const std::string& title,
                      const std::string& subtitle);

    // ── Helpers (Member B added) ─────────────────────────────
    void  initButtons();
    void  updateHUD();

    // ── Layout constants (original — kept exactly) ────────────
    static constexpr unsigned WIN_W   = 600u;
    static constexpr unsigned WIN_H   = 780u;
    static constexpr float    BOARD_X = 42.f;
    static constexpr float    BOARD_Y = 125.f;

    // ── Layout constants (Member B added) ────────────────────
    static constexpr float TOOLBAR_Y = static_cast<float>(WIN_H) - 55.f;
    static constexpr float WARNING_Y = static_cast<float>(WIN_H) - 100.f;
};