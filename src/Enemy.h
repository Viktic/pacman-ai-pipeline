//
// Created by Viktor Brandmaier on 05.07.25.
//

#pragma once
#include "Entity.h"
#include <string>
#include <random>

//Enemy class is derived from Entity class
class Enemy: public Entity{

public:

    Enemy(const sf::Texture& _texture,  sf::Vector2f _spawnPosition);
    void move(float tileSize, const std::vector<std::string>& _grid, const std::unordered_set<sf::Vector2i, tool::sfVector2iHash>& _crossings) override;
    sf::Vector2f& getMomentum(); 

private:

    float m_speed;
    sf::Vector2f m_momentum;
    
    //static randomization device
    static std::mt19937 m_rng;
};



