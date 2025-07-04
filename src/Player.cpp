//
// Created by Viktor Brandmaier on 03.07.25.
//
#include "Player.h"

#include <iostream>
#include <SFML/Graphics/Sprite.hpp>
#include "SFML/Graphics/Texture.hpp"

Player::Player(const char* _filePath):
// in SMFL3 sprite needs to be initialized in the initializer list
    m_texture(_filePath),
    m_sprite(m_texture),
    m_health(100),
    m_positionX(0.0f),
    m_positionY(0.0f) {

    m_sprite.setScale({0.05f, 0.05f});
    m_sprite.setPosition({m_positionX, m_positionY});
}

sf::Sprite& Player::getSprite() {
    return m_sprite;
}

void Player::move(float _rateX,float _rateY) {
    float positionX = m_sprite.getPosition().x;
    float positionY = m_sprite.getPosition().y;
    float newPositionX = positionX + _rateX;
    float newPositionY = positionY + _rateY;

    m_sprite.setPosition({newPositionX, newPositionY});
}




