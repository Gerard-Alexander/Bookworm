
#include "Player.h"
#include "Tile.h"

// ────────────────────────────────────────────────────────────────────────────
//  Constructor
// ────────────────────────────────────────────────────────────────────────────
Player::Player(const std::string& name)
    : m_name(name) {}

// ────────────────────────────────────────────────────────────────────────────
//  calculateWordScore
//
//  base  = sum of Scrabble-style tile letter values
//  bonus = 0           for words < 5 letters
//        = len × 10    for words of 5 letters
//        = len × 20    for words of 6-7 letters
//        = len × 50    for words of 8+ letters
//  total = (base + bonus) × current level (multiplier)
// ────────────────────────────────────────────────────────────────────────────
int Player::calculateWordScore(const std::string& word, int tileValueSum) const {
    int len = static_cast<int>(word.size());

    int bonus = 0;
    if      (len >= 8) bonus = len * 50;
    else if (len >= 6) bonus = len * 20;
    else if (len >= 5) bonus = len * 10;

    return (tileValueSum + bonus) * m_level;
}

// ────────────────────────────────────────────────────────────────────────────
//  addWord  –  awards points, records word, checks for level-up
// ────────────────────────────────────────────────────────────────────────────
void Player::addWord(const std::string& word, int tileValueSum) {
    int pts = calculateWordScore(word, tileValueSum);
    m_score += pts;
    m_wordHistory.push_back(word);
    ++m_wordsThisLevel;

    addXP(pts);   // <-- award XP based on points
    checkLevelUp();
}

// ────────────────────────────────────────────────────────────────────────────
//  checkLevelUp  –  advance level every WORDS_PER_LEVEL valid words
// ────────────────────────────────────────────────────────────────────────────
void Player::checkLevelUp() {
    if (m_xp >= m_xpToNextLevel) {
        m_xp -= m_xpToNextLevel;
        ++m_level;
        m_wordsThisLevel = 0;
    }
}

// ────────────────────────────────────────────────────────────
//  addXP
// ────────────────────────────────────────────────────────────
void Player::addXP(int amount) {
    m_xp += amount;
    checkLevelUp();
}

// ────────────────────────────────────────────────────────────
//  loseLife
// ────────────────────────────────────────────────────────────
void Player::loseLife() {
    if (m_lives > 0) --m_lives;
}

// ────────────────────────────────────────────────────────────────────────────
//  reset  –  start a fresh game
// ────────────────────────────────────────────────────────────────────────────
void Player::reset() {
    m_score          = 0;
    m_level          = 1;
    m_wordsThisLevel = 0;
    m_wordHistory.clear();
    m_lives          = 5;   // e.g. 3
    m_xp             = 0;
}
