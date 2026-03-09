#pragma once
#include <string>
#include <unordered_set>

// ============================================================
//  Dictionary  –  fast word-validity checker
//
//  Knows:
//    • A set of valid English words (loaded from file or built-in)
//    • The minimum word length rule (>= 3 letters)
//
//  Can:
//    • Load a plain-text word list (one word per line)
//    • Fall back to a built-in list when no file is found
//    • Answer isValid(word) in O(1) average time
//
//  Used by:
//    GameEngine calls isValid() before awarding points.
// ============================================================
class Dictionary {
public:
    static constexpr int MIN_WORD_LENGTH = 3;

    // Load from file; returns false if file cannot be opened
    bool loadFromFile(const std::string& path);

    // Load small built-in word list (fallback / testing)
    void loadBuiltIn();

    // Returns true when word meets length rule AND is in the list
    bool isValid(const std::string& word) const;

    std::size_t size() const { return m_words.size(); }

private:
    std::unordered_set<std::string> m_words;   // stored in upper-case
};
