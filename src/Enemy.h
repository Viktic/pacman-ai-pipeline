//
// Created by Viktor Brandmaier on 05.07.25.
//

#pragma once
#include "Entity.h"
#include <string>

class Enemy: public Entity{

public:

    sf::Vector2u setStartingPosition(sf::Vector2u _windowSize);
    Enemy(const std::string& _texturePath, sf::Vector2u _windowSize);
    void move(float _rateX, float _rateY, sf::Vector2u _windowSize) override;

private:
    float m_speed;

};



