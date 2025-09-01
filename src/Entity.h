//
// Created by Viktor Brandmaier on 03.07.25.
//

#pragma once

#include "tool.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <string>
#include <unordered_set>


class Entity {

public:

    Entity(const std::string& _filePath, sf::Vector2f& _spawnPosition);
    sf::Sprite& getSprite();
    sf::Texture& getTexture();
    virtual void move(float _tileSize, const std::vector<std::string>& _grid, const std::unordered_set<sf::Vector2i, tool::sfVector2iHash>& _crossings) = 0;


private:

    sf::Texture m_texture;
    sf::Sprite m_sprite;
};



