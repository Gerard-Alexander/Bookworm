#include "GameEngine.h"
#include "Tile.h"
#include <iostream>
#include <string>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <fstream>

// ============================================================
//  Button  -  implementation
//  label is std::optional<sf::Text> so Button is
//  default-constructible despite sf::Text having no default ctor
// ============================================================
void Button::init(const sf::Font& font, const std::string& text,
                  sf::Vector2f pos, sf::Vector2f size, unsigned fontSize)
{
    shape.setSize(size);
    shape.setPosition(pos);
    shape.setFillColor(sf::Color(70, 50, 30));
    shape.setOutlineThickness(2.f);
    shape.setOutlineColor(sf::Color(180, 140, 60));

    // Construct sf::Text into the optional (same pattern as Tile)
    label.emplace(font, text, fontSize);
    label->setFillColor(sf::Color(255, 230, 150));

    // Centre the label inside the button shape
    sf::FloatRect lb = label->getLocalBounds();
    label->setOrigin({ lb.position.x + lb.size.x / 2.f,
                       lb.position.y + lb.size.y / 2.f });
    label->setPosition(pos + size / 2.f);
}

bool Button::contains(sf::Vector2f point) const {
    return shape.getGlobalBounds().contains(point);
}

void Button::setHover(bool h) {
    hovered = h;
    shape.setFillColor(h ? sf::Color(120, 88, 45)
                         : sf::Color(70,  50, 30));
    shape.setOutlineColor(h ? sf::Color(255, 200, 80)
                            : sf::Color(180, 140, 60));
}

void Button::draw(sf::RenderWindow& window) const {
    window.draw(shape);
    if (label.has_value())
        window.draw(*label);
}

// ============================================================
//  GameEngine  -  constructor
//
//  sf::Text members that require a font cannot be initialised
//  here because the font is not loaded yet at construction time.
//  We initialise them with a temporary empty font and rebuild
//  them properly inside run() after loadFont() succeeds.
// ============================================================
GameEngine::GameEngine()
    : m_window(sf::VideoMode({WIN_W, WIN_H}),
               "Orthography",
               sf::Style::Titlebar | sf::Style::Close),
      m_grid(m_font, {BOARD_X, BOARD_Y}),
      m_scoreText  (m_font, "Score: 0", 18u),
      m_levelText  (m_font, "Level: 1", 18u),
      m_wordText   (m_font, "...",       26u),
      m_messageText(m_font, "",          24u),
      m_hintText   (m_font, "",          13u),
      m_titleText  (m_font, "ORTHOGRAPHY",  50u),
      m_warningText(m_font, "",          15u)
{
    m_window.setFramerateLimit(60);
}

// ============================================================
//  run()
// ============================================================
void GameEngine::run() {
    if (!loadFont()) {
        std::cerr << "[GameEngine] ERROR: No font found.\n"
                  << "  Place arial.ttf next to Bookworm.exe, or in assets/fonts/\n";
        return;
    }
    std::ifstream hsFile("assets/highscore.txt");
    if (hsFile.is_open()) {
        hsFile >> m_highScore;
        hsFile.close();
    }
    // Rebuild everything that needs a real font
    m_grid = Grid(m_font, {BOARD_X, BOARD_Y});

    if (!m_dictionary.loadFromFile("assets/words.txt")) {
        std::cerr << "[Dictionary] words.txt not found - using built-in list.\n";
        m_dictionary.loadBuiltIn();
    }
    std::cout << "[Dictionary] " << m_dictionary.size() << " words loaded.\n";

    if (!m_backgroundMusic.openFromFile("assets/music/Sakura-Girl-Cat-Walk-chosic.mp3")) {
        std::cerr << "[Audio] ERROR: Could not load background music.\n";
    } else {
        m_backgroundMusic.setLooping(true);    
        m_backgroundMusic.setVolume(30.f); // 0–100
        m_backgroundMusic.play();
    }

    if (!m_clickBuffer.loadFromFile("assets/sfx/spinopel-ceramic-tile-411505.mp3")) {
        std::cerr << "[Audio] ERROR: Could not load click sound.\n";
    } else {
        m_clickSound.emplace(m_clickBuffer);   
        m_clickSound->setVolume (30.f);          
    }

    if (!m_damageBuffer.loadFromFile("assets/sfx/Jewel.mp3")) {
        std::cerr << "[Audio] ERROR: Could not load damage sound.\n";
    } else {
        m_damageSound.emplace(m_damageBuffer);
        m_damageSound->setVolume(80.f); // adjust volume as needed
    }
    initHUDText();
    initButtons();

    sf::Clock clock;
    while (m_window.isOpen()) {
        float dt = clock.restart().asSeconds();
        m_totalTime += dt;

        processEvents();
        update(dt);
        render();
    }
    
}

// ============================================================
//  processEvents
// ============================================================
void GameEngine::processEvents() {
    while (const auto event = m_window.pollEvent()) {

        if (event->is<sf::Event::Closed>()) {
            m_window.close();
            return;
        }

        if (const auto* mm = event->getIf<sf::Event::MouseMoved>())
            onMouseMoved({ static_cast<float>(mm->position.x),
                           static_cast<float>(mm->position.y) });

        if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>())
            if (mb->button == sf::Mouse::Button::Left)
                onMousePressed({ static_cast<float>(mb->position.x),
                                  static_cast<float>(mb->position.y) });

        if (const auto* kb = event->getIf<sf::Event::KeyPressed>())
            onKeyPressed(kb->code);
    }
}

// ============================================================
//  update(dt)
// ============================================================
void GameEngine::update(float dt) {
    // Fade feedback message
    if (m_messageTimer > 0.f) {
        m_messageTimer -= dt;
        float alpha = (m_messageTimer < 0.4f)
                      ? std::max(0.f, m_messageTimer / 0.4f)
                      : 1.f;
        sf::Color c = m_messageText.getFillColor();
        c.a = static_cast<uint8_t>(255 * alpha);
        m_messageText.setFillColor(c);
    }

    if (m_state != GameState::Playing) return;

    // Game-over checks
    if (m_grid.hasExploded()) {
    m_player.loseLife();
    showMessage("A burning tile exploded! -1 life", 2.0f);
    m_grid.clearExplodedTiles();

    if (m_damageSound.has_value()) {
        m_damageSound->play();
    }

    if (!m_player.isAlive()) {
        saveHighScore();   // <-- call here
        m_state = GameState::GameOver;
        return;
    }
}
    // DFS possible-word check
    m_noPossibleWord = !m_grid.hasPossibleWord(m_dictionary);

    // Sync HUD
    updateHUD();
}
void GameEngine::saveHighScore() {
    if (m_player.getScore() > m_highScore) {
        m_highScore = m_player.getScore();
        std::ofstream hsFile("assets/highscore.txt");
        if (hsFile.is_open()) {
            hsFile << m_highScore;
            hsFile.close();
        }
    }
}
// ============================================================
//  render()
// ============================================================
void GameEngine::render() {
    m_window.clear(sf::Color(35, 22, 12));

    switch (m_state) {
        case GameState::Menu:
            renderMenu();
            break;
        case GameState::Playing:
            renderGame();
            break;
        case GameState::GameOver:
            renderGame();
            renderGameOverOverlay();
            break;
        case GameState::Help:    
        renderHelp(); break;
    }

    m_window.display();
}

// ============================================================
//  renderMenu()
// ============================================================
void GameEngine::renderMenu() {
    m_window.draw(m_titleText);

    sf::Text sub(m_font, "A Word Puzzle Adventure", 20u);
    sub.setFillColor(sf::Color(200, 170, 110));
    {
        sf::FloatRect b = sub.getLocalBounds();
        sub.setOrigin({ b.position.x + b.size.x / 2.f, 0.f });
        sub.setPosition({ WIN_W / 2.f, 175.f });
    }
    m_window.draw(sub);

    sf::RectangleShape line({ 280.f, 2.f });
    line.setFillColor(sf::Color(150, 110, 50));
    line.setPosition({ WIN_W / 2.f - 140.f, 210.f });
    m_window.draw(line);

    sf::Text howto(m_font,
        "Click adjacent tiles to spell words.\n"
        "Submit with ENTER or the Submit button.\n"
        "Burning tiles explode after 3 words -\n"
        "use them in a word before time runs out!",
        14u);
    howto.setFillColor(sf::Color(175, 155, 115));
    howto.setLineSpacing(1.4f);
    {
        sf::FloatRect b = howto.getLocalBounds();
        howto.setOrigin({ b.position.x + b.size.x / 2.f, 0.f });
        howto.setPosition({ WIN_W / 2.f, 225.f });
    }
    m_window.draw(howto);

    m_btnPlay    .draw(m_window);
    m_btnMenuExit.draw(m_window);

    sf::Text footer(m_font, "Longer words earn more points!", 12u);
    footer.setFillColor(sf::Color(110, 90, 60));
    {
        sf::FloatRect b = footer.getLocalBounds();
        footer.setOrigin({ b.position.x + b.size.x / 2.f, 0.f });
        footer.setPosition({ WIN_W / 2.f,
                              static_cast<float>(WIN_H) - 26.f });
    }
    m_window.draw(footer);

    sf::Text hsText(m_font, "High Score: " + std::to_string(m_highScore), 20u);
hsText.setFillColor(sf::Color(255, 200, 100));
{
    sf::FloatRect b = hsText.getLocalBounds();
    hsText.setOrigin({ b.position.x + b.size.x / 2.f, 0.f });
    hsText.setPosition({ WIN_W / 2.f, 310.f });
}
m_window.draw(hsText);
}

// ============================================================
//  renderGame()
// ============================================================
void GameEngine::renderGame() {
    
    m_grid.draw(m_window, m_totalTime);
    drawSidebar();              // draws the dark panel first

    // Draw title ON TOP of the panel
    sf::Text topLabel(m_font, "ORTHOGRAPHY", 22u);
    topLabel.setFillColor(sf::Color(255, 200, 50));
    topLabel.setStyle(sf::Text::Bold);
    topLabel.setPosition({ 10.f, 5.f });
    m_window.draw(topLabel); 

    m_grid.draw(m_window, m_totalTime);

    drawSidebar();

    m_window.draw(m_wordText);

    m_btnSubmit .draw(m_window);
    m_btnClear  .draw(m_window);
    m_btnNewGame.draw(m_window);
    m_btnExit   .draw(m_window);

    if (m_messageTimer > 0.f)
        m_window.draw(m_messageText);

    if (m_noPossibleWord)
        m_window.draw(m_warningText);
    
    m_btnHelp.draw(m_window);
}

// ============================================================
//  renderGameOverOverlay()
// ============================================================
void GameEngine::renderGameOverOverlay() {
    std::string sub =
        "Final Score: " + std::to_string(m_player.getScore()) +
        "   |   Level: "  + std::to_string(m_player.getLevel());
    drawOverlay("GAME OVER", sub);

    Button btnNG;
    btnNG.init(m_font, "Play Again",
               { WIN_W / 2.f - 85.f, WIN_H / 2.f + 65.f },
               { 170.f, 44.f }, 17u);

    sf::Vector2i mp = sf::Mouse::getPosition(m_window);
    btnNG.setHover(btnNG.contains({
        static_cast<float>(mp.x), static_cast<float>(mp.y) }));
    btnNG.draw(m_window);
}
void GameEngine::playClickSound() {
    if (m_clickSound.has_value()) {
        m_clickSound->play();
    }
}

// ============================================================
//  drawSidebar()
//  Top HUD panel (y = 40..135) — Member C fills in the body
// ============================================================
void GameEngine::drawSidebar() {
    sf::RectangleShape topPanel({ static_cast<float>(WIN_W), 88.f });
    topPanel.setPosition({ 0.f, 30.f });
    topPanel.setFillColor(sf::Color(50, 32, 15));
    topPanel.setOutlineThickness(1.f);
    topPanel.setOutlineColor(sf::Color(100, 70, 30));
    m_window.draw(topPanel);

    // Score + Level
    m_scoreText.setPosition({ 10.f, 42.f });
    m_levelText.setPosition({ 10.f, 68.f });
    m_window.draw(m_scoreText);
    m_window.draw(m_levelText);

    // Lives (hearts)
    for (int i = 0; i < m_player.getLives(); ++i) {
        sf::CircleShape heart(8.f);
        heart.setFillColor(sf::Color::Red);
        heart.setPosition({ 200.f + i * 20.f, 45.f });
        m_window.draw(heart);
    }

    // XP Bar
    sf::RectangleShape xpBg({ 120.f, 12.f });
    xpBg.setFillColor(sf::Color(80, 60, 40));
    xpBg.setPosition({ 200.f, 70.f });
    m_window.draw(xpBg);

    float ratio = static_cast<float>(m_player.getXP()) / m_player.getXPToNextLevel();
    sf::RectangleShape xpFill({ ratio * 120.f, 12.f });
    xpFill.setFillColor(sf::Color(100, 200, 100));
    xpFill.setPosition({ 200.f, 70.f });
    m_window.draw(xpFill);

    sf::Text burnCounterText(m_font,
        "Burning Tiles: " + std::to_string(m_grid.getBurnCount()), 16u);
    burnCounterText.setFillColor(sf::Color(255, 200, 100));
    burnCounterText.setPosition({ 360.f, 75.f });
    m_window.draw(burnCounterText);
}
// ============================================================
//  updateHUD()
// ============================================================
void GameEngine::updateHUD() {
    m_scoreText.setString("Score: " + std::to_string(m_player.getScore()));
    m_levelText.setString("Level: " + std::to_string(m_player.getLevel()));

    std::string word = m_grid.getSelectedWord();
    m_wordText.setString(word.empty() ? "..." : word);
    {
        sf::FloatRect wb = m_wordText.getLocalBounds();
        m_wordText.setOrigin({ wb.position.x + wb.size.x / 2.f,
                               wb.position.y + wb.size.y / 2.f });
        m_wordText.setPosition({ WIN_W / 2.f, TOOLBAR_Y - 30.f });
    }
}

// ============================================================
//  onMouseMoved()
// ============================================================
void GameEngine::onMouseMoved(sf::Vector2f pos) {
    m_btnSubmit .setHover(m_btnSubmit .contains(pos));
    m_btnClear  .setHover(m_btnClear  .contains(pos));
    m_btnNewGame.setHover(m_btnNewGame.contains(pos));
    m_btnExit   .setHover(m_btnExit   .contains(pos));
    m_btnPlay    .setHover(m_btnPlay    .contains(pos));
    m_btnHelp.setHover(m_btnHelp.contains(pos));
    m_btnMenuExit.setHover(m_btnMenuExit.contains(pos));
}

// ============================================================
//  onMousePressed()
// ============================================================
void GameEngine::onMousePressed(sf::Vector2f pos) {
    playClickSound();
    if (m_state == GameState::Menu) {
        if (m_btnPlay.contains(pos)) {
            resetGame();
            m_state = GameState::Playing;
        }
        if (m_btnMenuExit.contains(pos))
            m_window.close();
        return;
    }

    if (m_state == GameState::Help) {
        sf::FloatRect backArea({WIN_W / 2.f - 85.f, WIN_H - 100.f}, {170.f, 44.f});
        if (backArea.contains(pos)) {
            m_state = GameState::Playing; // or Menu, depending on where you want to return
        }
        return;
    }

    if (m_state == GameState::GameOver) {
        sf::FloatRect ngArea(
            { WIN_W / 2.f - 85.f, WIN_H / 2.f + 65.f }, { 170.f, 44.f });
        if (ngArea.contains(pos)) {
            resetGame();
            m_state = GameState::Playing;
        }
        return;
    }

    // Playing: toolbar buttons first
    if (m_btnSubmit .contains(pos)) { trySubmitWord();             return; }
    if (m_btnClear  .contains(pos)) { m_grid.clearSelection();     return; }
    if (m_btnNewGame.contains(pos)) { resetGame();                  return; }
    if (m_btnExit.contains(pos)) {
        saveHighScore();      
        m_state = GameState::Menu;
        m_grid = Grid(m_font, {BOARD_X, BOARD_Y});
        m_player.reset();
        return;
    }
    
    m_grid.onMousePressed(pos);
}

// ============================================================
//  onKeyPressed()
// ============================================================
void GameEngine::onKeyPressed(sf::Keyboard::Key key) {
    switch (key) {
        case sf::Keyboard::Key::Enter:
            if (m_state == GameState::Playing) trySubmitWord();
            break;
        case sf::Keyboard::Key::Backspace:
            if (m_state == GameState::Playing) m_grid.clearSelection();
            break;
        case sf::Keyboard::Key::R:
            resetGame();
            m_state = GameState::Playing;
            break;
        default:
            break;
    }
}
void GameEngine::renderHelp() {

    sf::Text title(m_font, "HOW TO PLAY", 36u);
    title.setFillColor(sf::Color(255, 80, 80)); // red
    title.setStyle(sf::Text::Bold);
    {
        sf::FloatRect tb = title.getLocalBounds();
        title.setOrigin({ tb.position.x + tb.size.x / 2.f,
                          tb.position.y + tb.size.y / 2.f });
        title.setPosition({ WIN_W / 2.f, 120.f }); // place near top
    }
    m_window.draw(title);

    sf::Text instructions(m_font,
        "Click adjacent tiles to spell words.\n"
        "Press ENTER or Submit to confirm.\n"
        "Burning tiles must be used before they explode!\n"
        "Longer words earn more points.\n"
        "Lives are lost when burning tiles explode.\n"
        "Level up by finding enough words.",
        16u);

    instructions.setFillColor(sf::Color(220, 220, 220));
    instructions.setLineSpacing(1.4f);
    {
        sf::FloatRect b = instructions.getLocalBounds();
        instructions.setOrigin({ b.position.x + b.size.x / 2.f, 0.f });
        instructions.setPosition({ WIN_W / 2.f, 200.f });
    }
    m_window.draw(instructions);

    // Add a "Back" button
    Button btnBack;
    btnBack.init(m_font, "Back", {WIN_W / 2.f - 85.f, WIN_H - 100.f}, {170.f, 44.f}, 18u);
    sf::Vector2i mp = sf::Mouse::getPosition(m_window);
    btnBack.setHover(btnBack.contains({static_cast<float>(mp.x), static_cast<float>(mp.y)}));
    btnBack.draw(m_window);
}

// ============================================================
//  trySubmitWord()
// ============================================================
void GameEngine::trySubmitWord() {
    std::string word = m_grid.getSelectedWord();
    if (word.empty()) return;

    if (m_dictionary.isValid(word)) {
        // Sum the point value of every selected tile
        int tileSum = 0;
        for (const Tile* t : m_grid.getSelectedTiles())
            tileSum += Tile::letterValue(t->getLetter());

        // Save level BEFORE adding the word so we can detect a level-up
        int prevLevel = m_player.getLevel();

        // Score via Player, then remove tiles from the board
        int pts = m_player.calculateWordScore(word, tileSum);
        m_player.addWord(word, tileSum);
        int currentLevel = m_player.getLevel();
        m_grid.removeSelectedTiles(currentLevel);

        // Level-up event: spawn 1 burning tile
        if (m_player.getLevel() > prevLevel) {
            showMessage("LEVEL UP!  +" + std::to_string(pts), 2.2f);
            m_grid.spawnBurningTile(1);   // only 1 tile on level-up
        } else {
            // Pick a praise message based on the score earned
            std::string msg;
            if      (pts >= 300) msg = "OUTSTANDING! +" + std::to_string(pts);
            else if (pts >= 150) msg = "Excellent!  +"  + std::to_string(pts);
            else if (pts >= 60)  msg = "Nice word!  +"  + std::to_string(pts);
            else                 msg = "+"               + std::to_string(pts);
            showMessage(msg, 1.6f);
        }

        std::cout << "[Word] " << word << " => " << pts << " pts  "
                  << "(total: " << m_player.getScore() << ")\n";
    } else {
        showMessage("Not a word!", 1.2f);
        m_grid.clearSelection();
    }
}

// ============================================================
//  resetGame()
// ============================================================
void GameEngine::resetGame() {
    m_player.reset();
    m_grid           = Grid(m_font, {BOARD_X, BOARD_Y});
    m_totalTime      = 0.f;
    m_noPossibleWord = false;
    showMessage("Good luck!", 1.5f);
}

// ============================================================
//  initButtons()
// ============================================================
void GameEngine::initButtons() {
    // 5 toolbar buttons evenly spaced along the bottom
    const float btnH = 44.f;
    const float btnW = 108.f;
    const float gap  =   6.f;
    float       x    =  18.f;

    m_btnSubmit .init(m_font, "Submit",   {x, TOOLBAR_Y}, {btnW, btnH}); x += btnW + gap;
    m_btnClear  .init(m_font, "Clear",    {x, TOOLBAR_Y}, {btnW, btnH}); x += btnW + gap;
    m_btnNewGame.init(m_font, "New Game", {x, TOOLBAR_Y}, {btnW, btnH}); x += btnW + gap;
    m_btnHelp.init(m_font, "?", {WIN_W - 50.f, 10.f}, {40.f, 40.f}, 22u);
    m_btnExit   .init(m_font, "Exit",     {x, TOOLBAR_Y}, {btnW, btnH});

    // 2 menu screen buttons centred
    const float mW = 220.f, mH = 52.f;
    const float mX = WIN_W / 2.f - mW / 2.f;
    m_btnPlay    .init(m_font, "Play Game", {mX, WIN_H / 2.f},        {mW, mH}, 20u);
    m_btnMenuExit.init(m_font, "Exit",      {mX, WIN_H / 2.f + 68.f}, {mW, mH}, 20u);
}

// ============================================================
//  initHUDText()  -  original method name kept
// ============================================================
void GameEngine::initHUDText() {
    m_titleText = sf::Text(m_font, "ORTHOGRAPHY", 58u);
    m_titleText.setFillColor(sf::Color(255, 200, 50));
    m_titleText.setStyle(sf::Text::Bold);
    {
        sf::FloatRect b = m_titleText.getLocalBounds();
        m_titleText.setOrigin({ b.position.x + b.size.x / 2.f, 0.f });
        m_titleText.setPosition({ WIN_W / 2.f, 80.f });
    }

    m_scoreText = sf::Text(m_font, "Score: 0", 18u);
    m_scoreText.setFillColor(sf::Color(255, 240, 180));

    m_levelText = sf::Text(m_font, "Level: 1", 18u);
    m_levelText.setFillColor(sf::Color(160, 255, 160));

    m_wordText = sf::Text(m_font, "...", 26u);
    m_wordText.setFillColor(sf::Color(255, 255, 255));
    m_wordText.setStyle(sf::Text::Bold);

    m_messageText = sf::Text(m_font, "", 24u);
    m_messageText.setFillColor(sf::Color(255, 220, 50));
    m_messageText.setStyle(sf::Text::Bold);

    m_hintText = sf::Text(m_font,
        "ENTER=submit  |  BACKSPACE=clear  |  R=restart",
        12u);
    m_hintText.setFillColor(sf::Color(140, 115, 75));
    {
        sf::FloatRect b = m_hintText.getLocalBounds();
        m_hintText.setOrigin({ b.position.x + b.size.x / 2.f, 0.f });
        m_hintText.setPosition({ WIN_W / 2.f,
                                   static_cast<float>(WIN_H) - 20.f });
    }

    m_warningText = sf::Text(m_font,
        "WARNING: No possible word on the board!", 14u);
    m_warningText.setFillColor(sf::Color(255, 70, 70));
    {
        sf::FloatRect b = m_warningText.getLocalBounds();
        m_warningText.setOrigin({ b.position.x + b.size.x / 2.f, 0.f });
        m_warningText.setPosition({ WIN_W / 2.f, WARNING_Y });
    }
}


// ============================================================
//  showMessage()
// ============================================================
void GameEngine::showMessage(const std::string& msg, float duration) {
    m_messageText.setString(msg);
    {
        sf::FloatRect b = m_messageText.getLocalBounds();
        m_messageText.setOrigin({ b.position.x + b.size.x / 2.f,
                                   b.position.y + b.size.y / 2.f });
        m_messageText.setPosition({ WIN_W / 2.f, BOARD_Y - 28.f });
    }
    sf::Color c = m_messageText.getFillColor();
    c.a = 255;
    m_messageText.setFillColor(c);
    m_messageTimer = duration;
}

// ============================================================
//  drawOverlay()
// ============================================================
void GameEngine::drawOverlay(const std::string& title,
                              const std::string& subtitle)
{
    sf::RectangleShape dim({ static_cast<float>(WIN_W),
                              static_cast<float>(WIN_H) });
    dim.setFillColor(sf::Color(0, 0, 0, 175));

    sf::Text t(m_font, title, 56u);
    t.setFillColor(sf::Color(255, 80, 80));
    t.setStyle(sf::Text::Bold);
    {
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin({ b.position.x + b.size.x / 2.f,
                      b.position.y + b.size.y / 2.f });
        t.setPosition({ WIN_W / 2.f, WIN_H / 2.f - 55.f });
    }
    m_window.draw(t);

    sf::Text s(m_font, subtitle, 19u);
    s.setFillColor(sf::Color(220, 220, 220));
    {
        sf::FloatRect b = s.getLocalBounds();
        s.setOrigin({ b.position.x + b.size.x / 2.f,
                      b.position.y + b.size.y / 2.f });
        s.setPosition({ WIN_W / 2.f, WIN_H / 2.f + 8.f });
    }
    m_window.draw(s);
}

// ============================================================
//  loadFont()
// ============================================================
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
    };
    for (const auto& p : candidates)
        if (m_font.openFromFile(p)) {
            std::cout << "[Font] Loaded: " << p << "\n";
            return true;
        }
    return false;
}