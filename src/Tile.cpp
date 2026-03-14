#include "Tile.h"
#include <cctype>
#include <cmath>

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
    if (m_state == TileState::Burning) {
        m_isPersistentBurn = true;
        m_burnCounter = MAX_BURN_STEPS;
    }

    if (s == TileState::Normal && m_isPersistentBurn) {
        m_state = TileState::Burning;
    } else {
        m_state = s;
    }

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
void Tile::draw(sf::RenderWindow& window, float totalTime) const {
    if (!m_text.has_value()) return;   // skip uninitialised default tiles

    // Create a mutable copy for animation
    sf::RectangleShape bg = m_background;

    if (m_state == TileState::Burning) {
        float pulse = (std::sin(totalTime * 12.f) + 1.f) / 2.f; // 0..1
        int r = 255;
        int g = 50 + static_cast<int>(pulse * 100);
        int b = 50;
        bg.setFillColor(sf::Color(r, g, b));
    }

    window.draw(bg);
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
            if (m_isPersistentBurn) {
                // A "Hot" selection - maybe a sickly lime green or orange-green
                m_background.setFillColor(sf::Color(180, 180, 0)); 
            } else {
                m_background.setFillColor(sf::Color(80, 190, 80)); // Normal Green
            }
            break;
        case TileState::Burning:
            int intensity = 255 - (m_burnCounter * 30); 
            m_background.setFillColor(sf::Color(255, 255 - intensity, 0)); // Shifts from Yellow to Red
            break;
    }
}

// for burning tiles

void Tile::tickBurn() {
    // Only tick down if the tile is actually in a Burning state
    if (m_isPersistentBurn && m_burnCounter > 0) {
        m_burnCounter--;
        
        // Update the visual representation (e.g., the text could show the number)
        if (m_text.has_value()) {
         
            m_text->setString(std::string(1, m_letter) + " " + std::to_string(m_burnCounter));
            
         
            sf::FloatRect tb = m_text->getLocalBounds();
            m_text->setOrigin({ 
                tb.position.x + tb.size.x / 2.f, 
                tb.position.y + tb.size.y / 2.f 
            });

           
            sf::Vector2f bgCenter = m_background.getPosition() + (m_background.getSize() / 2.f);
            m_text->setPosition(bgCenter);
        }
        
        updateColors();
    }
}

void Tile::resetBurnTimer() {
    m_state = TileState::Normal;
    m_burnCounter = 0;
    
    // Reset text to just the letter
    if (m_text) {
        m_text->setString(std::string(1, m_letter));
    }
    
    updateColors();
}

void Tile::setGridPos(int col, int row) {
    m_col = col;
    m_row = row;
}

void Tile::setPosition(sf::Vector2f pixelPos) {
    m_background.setPosition(pixelPos);
    
    centerText(); 
}

void Tile::centerText() {
    // Safety check: only proceed if we actually have text objects
    if (!m_text.has_value()) return;

    sf::Vector2f bgPos = m_background.getPosition();
    sf::Vector2f bgSize = m_background.getSize();
    sf::Vector2f bgCenter = bgPos + (bgSize / 2.f);

    // --- 1. Center the Main Letter ---
    sf::FloatRect textBounds = m_text->getLocalBounds();
    // Set origin to the center of the text's bounding box
    m_text->setOrigin({
        textBounds.position.x + textBounds.size.x / 2.f,
        textBounds.position.y + textBounds.size.y / 2.f
    });
    m_text->setPosition(bgCenter);
}




