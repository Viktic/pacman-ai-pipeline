//
// Created by Viktor Brandmaier on 19.08.25.
//

#pragma once
#include"SFML/System/Vector2.hpp"
#include <functional>

namespace tool {

    //define hash for std::Vector2i
    struct sfVector2iHash {
        size_t operator() (const sf::Vector2i _vector) const noexcept {
            size_t h1 =  std::hash<size_t>()( _vector.x);
            size_t h2 = std::hash<size_t>() (_vector.y);

            //combined hash with bit shifting 
            return h1 ^ (h2 << 1);
        }
    };
}
