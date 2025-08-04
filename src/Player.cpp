//
// Created by Viktor Brandmaier on 04.07.25.
//

#include "Player.h"
#include "Game.h"

Player::Player(const std::string &_textureFilePath, float _posX, float _posY):
Entity(_textureFilePath, _posX, _posY),
m_health(100),
m_speed(3.0f){

}


void Player::setHealth(int _value) {
    m_health = _value;
}

int Player::getHealth() {
    return m_health;
}

void Player::move(float _rateX, float _rateY, unsigned _borderX, unsigned _borderY) {

    float positionX = getSprite().getPosition().x;
    float positionY = getSprite().getPosition().y;
    float newPositionX = positionX + _rateX;
    float newPositionY = positionY + _rateY;

    if ((0 <= newPositionX and newPositionX <= _borderX-90) and (0 <= newPositionY and newPositionY <= _borderY-90)) {
        getSprite().setPosition({newPositionX, newPositionY});
    }

}


void Player::handleInput(unsigned _borderX, unsigned _borderY) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
        move(0.0f, -m_speed, _borderX, _borderY);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
        move(-m_speed, 0.0, _borderX, _borderY);

    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
        move(0.0f, m_speed,_borderX, _borderY);

    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
        move(m_speed, 0.0f,_borderX, _borderY);
    }
}

