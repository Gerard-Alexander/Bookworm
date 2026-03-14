#include "GameEngine.h"
#include "Tile.h"
#include <iostream>
#include <string>

// ────────────────────────────────────────────────────────────────────────────
//  Constructor
//  NOTE: Grid and UI require a valid font, so we pass a default-constructed
//        font here and rebuild both objects inside run() after loadFont().
// ────────────────────────────────────────────────────────────────────────────
GameEngine::GameEngine()
    : m_window(sf::VideoMode({WIN_W, WIN_H}),
               "Bookworm",
               sf::Style::Titlebar | sf::Style::Close),
      m_grid(m_font, {BOARD_X, BOARD_Y}),
      m_scoreText(m_font, "Score: 0",    22u),
      m_levelText(m_font, "Level: 1",    22u),
      m_wordText (m_font, "",            30u),
      m_messageText(m_font, "",          28u),
      m_hintText (m_font,
          "Click tiles to spell  |  ENTER = submit  |  BACKSPACE = clear  |  ESC = pause  |  R = restart",
          11u),
      m_titleText(m_font, "BOOKWORM", 40u)
{
    m_window.setFramerateLimit(60);
}

// ════════════════════════════════════════════════════════════════════════════
//  run()  –  public entry point; opens the window and drives the game loop
// ════════════════════════════════════════════════════════════════════════════
void GameEngine::run() {
    // 1. Load font first – everything text-related depends on it
    if (!loadFont()) {
        std::cerr << "[GameEngine] ERROR: Could not load any font.\n"
                  << "  Place a .ttf file such as 'arial.ttf' in the same\n"
                  << "  folder as the executable, or in assets/fonts/.\n";
        return;
    }

    // 2. Rebuild objects that need a valid font
    m_grid = Grid(m_font, {BOARD_X, BOARD_Y});
    initHUDText();

    // 3. Load dictionary
    if (!m_dictionary.loadFromFile("assets/words.txt")) {
        std::cerr << "[Dictionary] words.txt not found – using built-in word list.\n";
        m_dictionary.loadBuiltIn();
    }
    std::cout << "[Dictionary] Loaded " << m_dictionary.size() << " words.\n";

    // 4. Main game loop
    sf::Clock clock;
    while (m_window.isOpen()) {
        float dt = clock.restart().asSeconds();
        processEvents();
        update(dt);
        render();
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  processEvents()
//  Polls every pending SFML event and dispatches to the appropriate handler.
// ════════════════════════════════════════════════════════════════════════════
void GameEngine::processEvents() {
    while (const auto event = m_window.pollEvent()) {

        // ── Window close (X button) ────────────────────────────────────────
        if (event->is<sf::Event::Closed>()) {
            m_window.close();
            return;
        }

        // ── Left mouse button pressed ──────────────────────────────────────
        if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mb->button == sf::Mouse::Button::Left)
                onMousePressed({static_cast<float>(mb->position.x),
                                 static_cast<float>(mb->position.y)});
        }

        // ── Keyboard ──────────────────────────────────────────────────────
        if (const auto* kb = event->getIf<sf::Event::KeyPressed>())
            onKeyPressed(kb->code);
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  update(dt)
//  Per-frame logic: fade the feedback message, sync HUD text.
// ════════════════════════════════════════════════════════════════════════════
void GameEngine::update(float dt) {
    m_totalTime += dt;
    // Countdown and fade the feedback message
    if (m_messageTimer > 0.f) {
        m_messageTimer -= dt;
        float alpha = (m_messageTimer < 0.4f)
                      ? std::max(0.f, m_messageTimer / 0.4f)
                      : 1.f;
        sf::Color c = m_messageText.getFillColor();
        c.a = static_cast<uint8_t>(255 * alpha);
        m_messageText.setFillColor(c);
    }

    if (m_state == GameState::Playing) {
        // Sync score / level labels
        m_scoreText.setString("Score: " + std::to_string(m_player.getScore()));
        m_levelText.setString("Level: " + std::to_string(m_player.getLevel()));

        // Sync current-word display and re-centre it
        std::string word = m_grid.getSelectedWord();
        m_wordText.setString(word.empty() ? "..." : word);
        sf::FloatRect tb = m_wordText.getLocalBounds();
        m_wordText.setOrigin({tb.position.x + tb.size.x / 2.f,
                               tb.position.y + tb.size.y / 2.f});
        m_wordText.setPosition({WIN_W / 2.f,
                                 static_cast<float>(WIN_H) - 58.f});
                                  if (m_grid.hasExploded()) {
        m_state = GameState::GameOver;
        showMessage("A TILE EXPLODED!", 5.0f);
    }
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  render()
//  Draws every visual layer in order.
// ════════════════════════════════════════════════════════════════════════════
void GameEngine::render() {
    m_window.clear(sf::Color(45, 30, 20));   // dark wood background

    // ── Title bar ──────────────────────────────────────────────────────────
    m_window.draw(m_titleText);

    // ── Hint bar at the bottom ─────────────────────────────────────────────
    m_window.draw(m_hintText);

    // ── The 4×4 tile grid ──────────────────────────────────────────────────
    m_grid.draw(m_window, m_totalTime);

    // ── HUD: score, level, current word, timed message ────────────────────
    m_window.draw(m_scoreText);
    m_window.draw(m_levelText);
    m_window.draw(m_wordText);
    if (m_messageTimer > 0.f)
        m_window.draw(m_messageText);

    // ── State-specific overlays ────────────────────────────────────────────
    if (m_state == GameState::GameOver)
        drawOverlay("GAME OVER",
                    "Score: " + std::to_string(m_player.getScore()) +
                    "   |   Press R to restart");
    else if (m_state == GameState::Paused)
        drawOverlay("PAUSED", "Press ESC to resume   |   R to restart");

    m_window.display();
}

// ════════════════════════════════════════════════════════════════════════════
//  onMousePressed(pos)
//  Forwards the click position to Grid when the game is in Playing state.
// ════════════════════════════════════════════════════════════════════════════
void GameEngine::onMousePressed(sf::Vector2f pos) {
    if (m_state != GameState::Playing) return;
    m_grid.onMousePressed(pos);
}

// ════════════════════════════════════════════════════════════════════════════
//  onKeyPressed(key)
// ════════════════════════════════════════════════════════════════════════════
void GameEngine::onKeyPressed(sf::Keyboard::Key key) {
    switch (key) {

    // ── ENTER: submit current selection ───────────────────────────────────
    case sf::Keyboard::Key::Enter:
        if (m_state == GameState::Playing)
            trySubmitWord();
        break;

    // ── BACKSPACE: clear the whole selection ──────────────────────────────
    case sf::Keyboard::Key::Backspace:
        if (m_state == GameState::Playing)
            m_grid.clearSelection();
        break;

    // ── ESC: toggle pause / resume ────────────────────────────────────────
    case sf::Keyboard::Key::Escape:
        if      (m_state == GameState::Playing) m_state = GameState::Paused;
        else if (m_state == GameState::Paused)  m_state = GameState::Playing;
        break;

    // ── R: restart game ───────────────────────────────────────────────────
    case sf::Keyboard::Key::R:
        resetGame();
        break;

    default:
        break;
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  trySubmitWord()
//
//  Flow:
//    1. Get the word string from Grid
//    2. Ask Dictionary if it is valid
//    3a. Valid   → ask Player to score it, remove tiles from Grid, show feedback
//    3b. Invalid → show "Not a word!" and clear the selection
// ════════════════════════════════════════════════════════════════════════════
void GameEngine::trySubmitWord() {
    std::string word = m_grid.getSelectedWord();
    if (word.empty()) return;

    if (m_dictionary.isValid(word)) {
        // Sum the point value of every selected tile
        int tileSum = 0;
        for (const Tile* t : m_grid.getSelectedTiles())
            tileSum += Tile::letterValue(t->getLetter());

        // Score via Player, then remove tiles from the board
        int pts = m_player.calculateWordScore(word, tileSum);
        m_player.addWord(word, tileSum);
        int currentLevel = m_player.getLevel();
        m_grid.removeSelectedTiles(currentLevel);

        // Pick a praise message based on the score earned
        std::string msg;
        if      (pts >= 300) msg = "OUTSTANDING! +" + std::to_string(pts);
        else if (pts >= 150) msg = "Excellent!  +"  + std::to_string(pts);
        else if (pts >= 60)  msg = "Nice word!  +"  + std::to_string(pts);
        else                 msg = "+"               + std::to_string(pts);
        showMessage(msg, 1.6f);

        std::cout << "[Word] " << word << " => " << pts << " pts  "
                  << "(total: " << m_player.getScore() << ")\n";
    } else {
        showMessage("Not a word!", 1.2f);
        m_grid.clearSelection();
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  resetGame()
// ════════════════════════════════════════════════════════════════════════════
void GameEngine::resetGame() {
    m_player.reset();
    m_grid  = Grid(m_font, {BOARD_X, BOARD_Y});
    m_state = GameState::Playing;
    showMessage("New game — good luck!", 2.f);
}

// ════════════════════════════════════════════════════════════════════════════
//  loadFont()
//  Tries several common paths so it works on Windows, Linux, and macOS.
// ════════════════════════════════════════════════════════════════════════════
bool GameEngine::loadFont() {
    const std::vector<std::string> candidates = {
        "assets/fonts/Roboto-Regular.ttf",
        "assets/fonts/OpenSans-Regular.ttf",
        "assets/fonts/arial.ttf",
        "arial.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/calibri.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
    };
    for (const auto& path : candidates)
        if (m_font.openFromFile(path)) {
            std::cout << "[Font] Loaded: " << path << "\n";
            return true;
        }
    return false;
}

// ════════════════════════════════════════════════════════════════════════════
//  initHUDText()  –  configure all persistent sf::Text objects
// ════════════════════════════════════════════════════════════════════════════
void GameEngine::initHUDText() {
    // Title (top-centre)
    m_titleText = sf::Text(m_font, "BOOKWORM", 40u);
    m_titleText.setFillColor(sf::Color(255, 200, 50));
    m_titleText.setStyle(sf::Text::Bold);
    {
        sf::FloatRect b = m_titleText.getLocalBounds();
        m_titleText.setOrigin({b.position.x + b.size.x / 2.f, 0.f});
        m_titleText.setPosition({WIN_W / 2.f, 10.f});
    }

    // Score (top-right)
    m_scoreText = sf::Text(m_font, "Score: 0", 22u);
    m_scoreText.setFillColor(sf::Color(255, 240, 180));
    m_scoreText.setPosition({static_cast<float>(WIN_W) - 180.f, 10.f});

    // Level (below score)
    m_levelText = sf::Text(m_font, "Level: 1", 22u);
    m_levelText.setFillColor(sf::Color(160, 255, 160));
    m_levelText.setPosition({static_cast<float>(WIN_W) - 180.f, 38.f});

    // Current word (bottom-centre, re-centred every frame)
    m_wordText = sf::Text(m_font, "...", 30u);
    m_wordText.setFillColor(sf::Color(255, 255, 255));
    m_wordText.setStyle(sf::Text::Bold);

    // Feedback message (mid-top, fades out)
    m_messageText = sf::Text(m_font, "", 26u);
    m_messageText.setFillColor(sf::Color(255, 220, 50));
    m_messageText.setStyle(sf::Text::Bold);

    // Hint bar (bottom of window)
    m_hintText = sf::Text(m_font,
        "Click tiles  |  ENTER=submit  |  BACKSPACE=clear  |  ESC=pause  |  R=restart",
        12u);
    m_hintText.setFillColor(sf::Color(160, 140, 110));
    {
        sf::FloatRect b = m_hintText.getLocalBounds();
        m_hintText.setOrigin({b.position.x + b.size.x / 2.f, 0.f});
        m_hintText.setPosition({WIN_W / 2.f,
                                  static_cast<float>(WIN_H) - 22.f});
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  showMessage(msg, duration)
// ════════════════════════════════════════════════════════════════════════════
void GameEngine::showMessage(const std::string& msg, float duration) {
    m_messageText.setString(msg);
    sf::FloatRect b = m_messageText.getLocalBounds();
    m_messageText.setOrigin({b.position.x + b.size.x / 2.f,
                               b.position.y + b.size.y / 2.f});
    m_messageText.setPosition({WIN_W / 2.f, BOARD_Y - 32.f});
    sf::Color c = m_messageText.getFillColor();
    c.a = 255;
    m_messageText.setFillColor(c);
    m_messageTimer = duration;
}

// ════════════════════════════════════════════════════════════════════════════
//  drawOverlay(title, subtitle)
//  Semi-transparent black dim + centred title/subtitle text.
// ════════════════════════════════════════════════════════════════════════════
void GameEngine::drawOverlay(const std::string& title,
                              const std::string& subtitle)
{
    // Dim the whole screen
    sf::RectangleShape dim({static_cast<float>(WIN_W),
                             static_cast<float>(WIN_H)});
    dim.setFillColor(sf::Color(0, 0, 0, 170));
    m_window.draw(dim);

    // Big title
    sf::Text t(m_font, title, 52u);
    t.setFillColor(sf::Color(255, 80, 80));
    t.setStyle(sf::Text::Bold);
    {
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin({b.position.x + b.size.x / 2.f,
                     b.position.y + b.size.y / 2.f});
        t.setPosition({WIN_W / 2.f,
                        static_cast<float>(WIN_H) / 2.f - 40.f});
    }
    m_window.draw(t);

    // Subtitle
    sf::Text s(m_font, subtitle, 22u);
    s.setFillColor(sf::Color(220, 220, 220));
    {
        sf::FloatRect b = s.getLocalBounds();
        s.setOrigin({b.position.x + b.size.x / 2.f,
                     b.position.y + b.size.y / 2.f});
        s.setPosition({WIN_W / 2.f,
                        static_cast<float>(WIN_H) / 2.f + 20.f});
    }
    m_window.draw(s);
}
