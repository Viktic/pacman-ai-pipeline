//
// Created by Viktor Brandmaier on 05.07.25.
//

#include "Enemy.h"
#include <iostream>

//static randomization device initialization
std::mt19937 Enemy::m_rng(std::random_device{}());

//enemy constructor
Enemy::Enemy(const std::string& _texturePath, sf::Vector2f _spawnPosition):
m_speed(2.0f),
m_momentum({0.0f, 0.0f}),
Entity(_texturePath, _spawnPosition) {
    getSprite().setColor(sf::Color::Red); 
}

//momentum getter
sf::Vector2f& Enemy::getMomentum() {
    return m_momentum;
}

void Enemy::move(float _tileSize, const std::vector<std::string>& _grid, const std::unordered_set<sf::Vector2i, tool::sfVector2iHash>& _crossings)  {
    //current enemy position
    sf::Vector2f pos = getSprite().getPosition();

    //get the grid coordinates
    int col = int((pos.x - 0.5f * _tileSize) / _tileSize);
    int row = int((pos.y - 0.5f * _tileSize) / _tileSize);

    //directions vector to keep track of possible movement directions
    std::vector<sf::Vector2f> directions;
    directions.reserve(4);

    //tile center coordinates
    float centerX = col * _tileSize + 0.5f * _tileSize;
    float centerY = row * _tileSize + 0.5f * _tileSize;

    //check if current tile is a crossing
    float epsilon = m_speed;
    if (_crossings.find({col, row}) != _crossings.end() && std::abs(pos.x - centerX) < epsilon &&
    std::abs(pos.y - centerY) < epsilon){

        //char representation of all possible grid directions
        char up    = _grid[row-1][col];
        char down  = _grid[row+1][col];
        char left  = _grid[row][col-1];
        char right = _grid[row][col+1];

        //static declaration of direction vectors
        static constexpr sf::Vector2f momentumUp {0.0f,-1.0f};
        static constexpr sf::Vector2f momentumDown {0.0f,1.0f};
        static constexpr sf::Vector2f momentumLeft {-1.0f,0.0f};
        static constexpr sf::Vector2f momentumRight {1.0f,0.0f};

        //check if the direction is free && not the inverse of the current direction
        if (up != '#' && m_momentum != sf::Vector2f{0.0f, 1.0f}) {
            directions.push_back(momentumUp);
        }
        if (down != '#' && m_momentum != sf::Vector2f{0.0f, -1.0f}) {
            directions.push_back(momentumDown);
        }
        if (left != '#' && m_momentum != sf::Vector2f{1.0f, 0.0f}) {
            directions.push_back(momentumLeft);
        }
        if (right != '#' && m_momentum != sf::Vector2f{-1.0f, 0.0f}) {
            directions.push_back(momentumRight);
        }
    }

    //set initial impulse if enemy spawn position is a corridor
    else if (_crossings.find({col, row}) == _crossings.end() && m_momentum == sf::Vector2f{0.0f, 0.0f}) {

        //check if one direction is blocked (possible directions can only be orthogonal)
        char up = _grid[row-1][col];
        if (up == '#') {
            directions.push_back({1.0f, 0.0f});
            directions.push_back({-1.0f, 0.0f});
        } else {
            directions.push_back({0.0f, 1.0f});
            directions.push_back({0.0f, -1.0f});
        }
    }

    //randomly select a valid direction from the directions vector
    if (!directions.empty()) {
        std::uniform_int_distribution<int> dist(0, directions.size() - 1);
        int directionIndex = dist(m_rng);
        m_momentum = directions[directionIndex];
    }

    //update the player position
    sf::Vector2f newPos = {
        pos.x + m_momentum.x * m_speed,
        pos.y + m_momentum.y * m_speed
    };

    getSprite().setPosition(newPos);

}




