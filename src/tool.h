//
// Created by Viktor Brandmaier on 19.08.25.
//

#pragma once
#include"SFML/System/Vector2.hpp"
#include <functional>

namespace tool {
    struct sfVector2iHash {
        //creates hash for std::Vector2i
        size_t operator() (const sf::Vector2i _vector) const noexcept {
            size_t h1 =  std::hash<size_t>()( _vector.x);
            size_t h2 = std::hash<size_t>() (_vector.y);
            //combined hash
            return h1 ^ (h2 << 1);
        }
    };
}