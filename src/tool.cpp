#include "tool.h"

namespace tool {
    
    std::unordered_map<std::string, sf::Vector2f> translationMap = {
    
    {"[-1]", sf::Vector2f{-1.0f, -1.0f}},
    {"[0]", sf::Vector2f{0.0f, 0.0f}},
    {"[1]", sf::Vector2f{-1.0f, 0.0f}},
    {"[2]", sf::Vector2f{1.0f, 0.0f}},
    {"[3]", sf::Vector2f{0.0f, -1.0f}},
    {"[4]", sf::Vector2f{0.0f, 1.0f}}
    
    };
}