//
// Created by Viktor Brandmaier on 19.08.25.
//

#pragma once
#include <SFML/System/Vector2.hpp>
#include <functional>
#include <unordered_map>
#include <string>


namespace tool {
    
    //defines hash for std::Vector2i
    struct sfVector2iHash {
        size_t operator() (const sf::Vector2i _vector) const noexcept {
            size_t h1 =  std::hash<int>()( _vector.x);
            size_t h2 = std::hash<int>() (_vector.y);

            //combined hash with bit shifting 
            return h1 ^ (h2 << 1);
        }
    };

    //defines a translation hash-map to translate the ml-model response into direction vectors
    extern std::unordered_map<std::string, sf::Vector2f> translationMap; 
}
