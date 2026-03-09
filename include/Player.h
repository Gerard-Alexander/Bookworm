#pragma once
#include <string>
#include <vector>

// ============================================================
//  Player  –  score-keeping and progression
//
//  Knows:
//    • Current score, level, and words-submitted-this-level
//    • Full history of every accepted word
//    • Player's name
//
//  Can:
//    • Calculate a word's score (tile values + length bonus × level)
//    • Add a word and update the score
//    • Automatically level up after every N valid words
//    • Reset all state for a new game
//
//  Scoring formula:
//    base  = sum of letter point values for each tile used
//    bonus = wordLen × 10  (only for words >= 5 letters)
//          = wordLen × 20  (only for words >= 6 letters)
//          = wordLen × 50  (only for words >= 8 letters)
//    total = (base + bonus) × level
// ============================================================
class Player {
public:
    Player() = default;
    explicit Player(const std::string& name);

    // ── Scoring ──────────────────────────────────────────────
    int  calculateWordScore(const std::string& word, int tileValueSum) const;
    void addWord(const std::string& word, int tileValueSum);

    // ── Getters ──────────────────────────────────────────────
    int                             getScore()       const { return m_score;       }
    int                             getLevel()       const { return m_level;       }
    const std::string&              getName()        const { return m_name;        }
    const std::vector<std::string>& getWordHistory() const { return m_wordHistory; }

    // ── Progression ──────────────────────────────────────────
    void checkLevelUp();   // called internally after each word

    // ── Reset ────────────────────────────────────────────────
    void reset();

private:
    std::string              m_name            = "Player";
    int                      m_score           = 0;
    int                      m_level           = 1;
    int                      m_wordsThisLevel  = 0;
    std::vector<std::string> m_wordHistory;

    static constexpr int WORDS_PER_LEVEL = 5;
};
