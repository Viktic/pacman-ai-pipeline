//
// Created by Viktor Brandmaier on 05.07.25.
//

#include "Enemy.h"

//feature currently not needed
/*
std::mt19937 Enemy::m_rng(std::random_device{}());
*/



//feature currently nod needed
/*
sf::Vector2u Enemy::setStartingPosition(sf::Vector2u _windowSize) {

    std::uniform_int_distribution<std::mt19937::result_type> distX(87,_windowSize.x-87);
    std::uniform_int_distribution<std::mt19937::result_type> distY(87,_windowSize.y-87);
    sf::Vector2u spawnPosition = {distX(m_rng), distY(m_rng)};
    return spawnPosition;
}
*/


Enemy::Enemy(const std::string& _texturePath, sf::Vector2u _spawnPosition):
m_speed(2.0f),
Entity(_texturePath, _spawnPosition) {
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




