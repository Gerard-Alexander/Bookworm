#include "Tile.h"
#include <cctype>

// -- Scrabble-style letter point values ----------------------------------------
static const int LETTER_VALUES[26] = {
//  A  B  C  D  E  F  G  H  I  J   K  L  M  N  O  P  Q   R  S  T  U  V  W  X  Y   Z
    1, 3, 3, 2, 1, 4, 2, 4, 1, 8,  5, 1, 3, 1, 1, 3, 10, 1, 1, 1, 1, 4, 4, 8, 4, 10
};

// ------------------------------------------------------------------------------
//  Constructor
// ------------------------------------------------------------------------------
Tile::Tile(char letter, int col, int row,
           const sf::Font& font, sf::Vector2f pixelPos, float size)
    : m_letter(static_cast<char>(std::toupper(letter))),
      m_col(col), m_row(row)
{
    // Background rectangle
    m_background.setSize({size, size});
    m_background.setPosition(pixelPos);
    m_background.setOutlineThickness(2.f);
    m_background.setOutlineColor(sf::Color(80, 60, 40));

    // Construct sf::Text into the optional
    m_text.emplace(font,
                   std::string(1, static_cast<char>(std::toupper(letter))),
                   32u);

    // Centre the letter text inside the tile
    sf::FloatRect tb = m_text->getLocalBounds();
    m_text->setOrigin({ tb.position.x + tb.size.x / 2.f,
                        tb.position.y + tb.size.y / 2.f });
    m_text->setPosition(pixelPos + sf::Vector2f(size / 2.f, size / 2.f));

    updateColors();
}

// ------------------------------------------------------------------------------
//  State setters
// ------------------------------------------------------------------------------
void Tile::setState(TileState s) {
    m_state = s;
    updateColors();
}

void Tile::setLetter(char c) {
    m_letter = static_cast<char>(std::toupper(c));
    if (m_text.has_value()) {
        m_text->setString(std::string(1, m_letter));

        // Re-centre after the string changes
        sf::FloatRect tb = m_text->getLocalBounds();
        m_text->setOrigin({ tb.position.x + tb.size.x / 2.f,
                            tb.position.y + tb.size.y / 2.f });
    }
}

// ------------------------------------------------------------------------------
//  Hit-testing
// ------------------------------------------------------------------------------
bool Tile::contains(sf::Vector2f point) const {
    return m_background.getGlobalBounds().contains(point);
}

// ------------------------------------------------------------------------------
//  Rendering  -  only draw if fully initialised (has text)
// ------------------------------------------------------------------------------
void Tile::draw(sf::RenderWindow& window) const {
    if (!m_text.has_value()) return;   // skip uninitialised default tiles
    window.draw(m_background);
    window.draw(*m_text);
}

// ------------------------------------------------------------------------------
//  Static utility
// ------------------------------------------------------------------------------
int Tile::letterValue(char c) {
    c = static_cast<char>(std::toupper(c));
    if (c < 'A' || c > 'Z') return 0;
    return LETTER_VALUES[c - 'A'];
}

// ------------------------------------------------------------------------------
//  Private - recolour based on current state
// ------------------------------------------------------------------------------
void Tile::updateColors() {
    switch (m_state) {
        case TileState::Normal:
            m_background.setFillColor(sf::Color(210, 180, 140)); // tan
            if (m_text.has_value()) m_text->setFillColor(sf::Color(50, 30, 10));
            break;
        case TileState::Selected:
            m_background.setFillColor(sf::Color(80, 190, 80));   // green
            if (m_text.has_value()) m_text->setFillColor(sf::Color(255, 255, 255));
            break;
        case TileState::Burning:
            m_background.setFillColor(sf::Color(220, 70, 20));   // red-orange
            if (m_text.has_value()) m_text->setFillColor(sf::Color(255, 240, 200));
            break;
    }
}