//
// Created by Viktor Brandmaier on 04.07.25.
//

#include "Player.h"
#include "Game.h"
#include <SFML/System/Vector2.hpp>

Player* Player::player = nullptr;

Player::Player(const std::string &_textureFilePath):
Entity(_textureFilePath, {0,0}),
m_health(100),
m_speed(3.0f) {

}

Player* Player::get() {
    if (player == nullptr) {
        player = new Player("/Users/viktorbrandmaier/Desktop/Studium Programmieren/OOP_Game/src/sprites/HannesSprite.png");
    }
    return player;
}



void Player::setHealth(int _value) {
    m_health = _value;
}

int Player::getHealth() {
    return m_health;
}

void Player::move(float _rateX, float _rateY, sf::Vector2u _windowSize) {

    float positionX = getSprite().getPosition().x;
    float positionY = getSprite().getPosition().y;
    float newPositionX = positionX + _rateX;
    float newPositionY = positionY + _rateY;

    sf::Vector2f scale = getSprite().getScale();
    //WORK IN PROGRESS
    sf::Vector2u size = getTexture().getSize();
    float offsetRight = size.x * scale.x;
    float offsetBottom = size.y * scale.y;

    if ((0 <= newPositionX and newPositionX <= _windowSize.x-offsetRight) and (0 <= newPositionY and newPositionY <= _windowSize.y-offsetBottom)) {
        getSprite().setPosition({newPositionX, newPositionY});
    }
}


void Player::handleInput(sf::Vector2u _windowSize) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
        move(0.0f, -m_speed, _windowSize);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
        move(-m_speed, 0.0, _windowSize);

    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
        move(0.0f, m_speed,_windowSize);

    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
        move(m_speed, 0.0f,_windowSize);
    }
}

