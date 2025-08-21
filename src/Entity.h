//
// Created by Viktor Brandmaier on 03.07.25.
//

#pragma once

#include "tool.h"
#include <SFML/Graphics/Sprite.hpp>
#include "SFML/Graphics/Texture.hpp"
#include <string>
#include <unordered_set>


class Entity {

public:
    Entity(const std::string& _filePath, sf::Vector2u _spawnPosition);
    virtual ~Entity();

    sf::Sprite& getSprite();
    sf::Texture& getTexture();
    static size_t getEntityCount();
    virtual void move(float _tileSize, std::vector<std::string>*, std::unordered_set<sf::Vector2i, tool::sfVector2iHash>* _crossings) = 0;


private:
    static unsigned m_entityCount;
    sf::Texture m_texture;
    sf::Sprite m_sprite;
    float m_positionX;
    float m_positionY;


};



