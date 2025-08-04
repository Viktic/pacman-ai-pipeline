//
// Created by Viktor Brandmaier on 05.07.25.
//

#include "Enemy.h"
#include <random>






sf::Vector2u Enemy::setStartingPosition(sf::Vector2u _windowSize) {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> distX(87,_windowSize.x-87);
    std::uniform_int_distribution<std::mt19937::result_type> distY(87,_windowSize.y-87);
    sf::Vector2u spawnPosition = {distX(rng), distY(rng)};
    return spawnPosition;
}



Enemy::Enemy(const std::string& _texturePath, sf::Vector2u _windowSize):
m_speed(2.0f),
Entity(_texturePath, setStartingPosition(_windowSize)) {
}

void Enemy::move(float _rateX, float _rateY, sf::Vector2u _windowSize ) {
    {
        float positionX = getSprite().getPosition().x;
        float positionY = getSprite().getPosition().y;
        float newPositionX = positionX + _rateX;
        float newPositionY = positionY + _rateY;

        if (0 <= newPositionX and newPositionX <= _windowSize.x) {
            getSprite().setPosition({newPositionX, newPositionY});
        }
        if (0 <= newPositionY and newPositionX <= _windowSize.y) {
            getSprite().setPosition({newPositionX, newPositionY});
        }
    }
}




