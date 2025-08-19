//
// Created by Viktor Brandmaier on 05.07.25.
//

#pragma once
#include "Entity.h"
#include <string>
#include <random>

class Enemy: public Entity{

public:
    //feature currently not needed
    /*
    sf::Vector2u setStartingPosition(sf::Vector2u _windowSize);
    */

    Enemy(const std::string& _texturePath, sf::Vector2u _windowSize);
    void move(float tileSize, std::vector<std::string>*, std::unordered_set<sf::Vector2i, tool::sfVector2iHash>* _crossings) override;

private:
    float m_speed;
    //static randomization device
    //feature currently not needed
    /*
    static std::mt19937 m_rng;
    */
};



