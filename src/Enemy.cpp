//
// Created by Viktor Brandmaier on 05.07.25.
//

#include "Enemy.h"
#include <random>






float Enemy::setStartingPosition(float _dimension) {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(87,_dimension-87);
    return dist6(rng);
}


Enemy::Enemy(const std::string& _texturePath):
m_speed(2.0f),
Entity(_texturePath, setStartingPosition(1920),setStartingPosition(1080))
{
}

void Enemy::move(float _rateX, float _rateY, unsigned _borderX, unsigned _borderY) {
    {
        float positionX = getSprite().getPosition().x;
        float positionY = getSprite().getPosition().y;
        float newPositionX = positionX + _rateX;
        float newPositionY = positionY + _rateY;

        if (0 <= newPositionX and newPositionX <= _borderX) {
            getSprite().setPosition({newPositionX, newPositionY});
        }
        if (0 <= newPositionY and newPositionX <= _borderY) {
            getSprite().setPosition({newPositionX, newPositionY});
        }
    }
}




