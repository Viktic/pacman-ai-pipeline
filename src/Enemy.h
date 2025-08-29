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

    Enemy(const std::string& _texturePath, sf::Vector2f _spawnPosition);
    void move(float tileSize, const std::vector<std::string>& _grid, const std::unordered_set<sf::Vector2i, tool::sfVector2iHash>& _crossings) override;
    sf::Vector2f& getMomentum(); 


private:
    float m_speed;
    sf::Vector2f m_momentum;
    //static randomization device
    static std::mt19937 m_rng;
};



