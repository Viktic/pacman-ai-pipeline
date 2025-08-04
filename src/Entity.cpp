//
// Created by Viktor Brandmaier on 03.07.25.
//
#include "Entity.h"
#include <iostream>
#include <SFML/Graphics/Sprite.hpp>
#include "Game.h"
#include "SFML/Graphics/Texture.hpp"

unsigned Entity::m_entityCount = 0;


Entity::Entity(const std::string& _filePath, const float _posX, const float _posY):
// in SMFL3 sprite needs to be initialized in the initializer list
    m_texture(_filePath),
    m_sprite(m_texture),
    m_positionX(_posX),
    m_positionY(_posY) {

    m_sprite.setScale({0.05f, 0.05f});
    m_sprite.setPosition({m_positionX, m_positionY});
    // entity count is increased with every entity instance created
    ++m_entityCount;
}

Entity::~Entity() {
    --m_entityCount;
}


sf::Sprite& Entity::getSprite() {
    return m_sprite;
}

sf::Texture& Entity::getTexture() {
    return m_texture;
}


size_t Entity::getEntityCount() {
    return m_entityCount;
}




