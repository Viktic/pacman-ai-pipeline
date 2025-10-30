//
// Created by Viktor Brandmaier on 04.07.25.
//

#include "Player.h"
#include "Game.h"
#include <SFML/System/Vector2.hpp>
#include <iostream>
#include <cmath>


Player* Player::instance = nullptr; 

//Player constructor
Player::Player(const sf::Texture& _texture, sf::Vector2f _spawnPosition) :
    Entity(_texture ,_spawnPosition),
    m_speed(2.0f),
    m_momentum({ 0.0f,0.0f }),
    m_buffer({ 0.0f, 0.0f })
 {
    getSprite().setPosition(_spawnPosition);
}

//resets player momentum
void Player::resetMomentum() {
    m_momentum = { 0.0f, 0.0f };
}

//momentum getter
sf::Vector2f& Player::getMomentum() {
    return m_momentum;
}

sf::Vector2f& Player::getBuffer() {
    return m_buffer; 
}


 
//handle the buffered-movement logic 
void Player::move(float _tileSize, const std::vector<std::string>& _grid, const std::unordered_set<sf::Vector2i, tool::sfVector2iHash>& _crossings) {


    //current player position
    sf::Vector2f pos = getSprite().getPosition();

    //current player grid position
    int col = static_cast<int>(floor(pos.x / _tileSize));
    int row = static_cast<int>(floor(pos.y / _tileSize));

    //tile-center position
    float centerX = col * _tileSize + 0.5f * _tileSize;
    float centerY = row * _tileSize + 0.5f * _tileSize;

    //static direction vectors for one-time declaration
    static constexpr sf::Vector2f momentumUp{ 0.0f, -1.0f };
    static constexpr sf::Vector2f momentumDown{ 0.0f, 1.0f };
    static constexpr sf::Vector2f momentumLeft{ -1.0f, 0.0f };
    static constexpr sf::Vector2f momentumRight{ 1.0f, 0.0f };

    //check distance to center     
    float distanceToCenter = sqrt(pow(pos.x - centerX, 2) + pow(pos.y - centerY, 2));

    //check if the tileCenter will be crossed
    bool crossingCenter = false;
    static sf::Vector2f lastPos = pos;

    //check horizontal movement
    if (m_momentum.x != 0.0f) { 
        crossingCenter = (lastPos.x <= centerX && pos.x >= centerX) ||
            (lastPos.x >= centerX && pos.x <= centerX);
    }
    //check vertical movement
    else if (m_momentum.y != 0.0f) { 
        crossingCenter = (lastPos.y <= centerY && pos.y >= centerY) ||
            (lastPos.y >= centerY && pos.y <= centerY);
    }

    //update lastPos
    lastPos = pos;

    //tolerance 
    float epsilon = m_speed * 1.5f; 

    //check if current tile is a crossing and close proximity to center
    if (_crossings.find({ col, row }) != _crossings.end() && (distanceToCenter <= epsilon || crossingCenter)) {
        
        //keep track of direction change (only need to snap to center once)
        bool directionChanged = false;

        //check possible directions (with bounds checking)
        bool canGoUp = (row > 0) && (_grid[row - 1][col] != '#');
        bool canGoDown = (row < static_cast<int>(_grid.size() - 1)) && (_grid[row + 1][col] != '#');
        bool canGoLeft = (col > 0) && (_grid[row][col - 1] != '#');
        bool canGoRight = (col < static_cast<int>(_grid[row].size() - 1)) && (_grid[row][col + 1] != '#');

        //set starting impulse if not moving yet
        if (m_momentum.x == 0.0f && m_momentum.y == 0.0f &&
            (m_buffer.x != 0.0f || m_buffer.y != 0.0f)) {
            if (canGoUp && m_buffer == momentumUp) {
                m_momentum = momentumUp;
                directionChanged = true;
            }
            else if (canGoDown && m_buffer == momentumDown) {
                m_momentum = momentumDown;
                directionChanged = true;
            }
            else if (canGoLeft && m_buffer == momentumLeft) {
                m_momentum = momentumLeft;
                directionChanged = true;
            }
            else if (canGoRight && m_buffer == momentumRight) {
                m_momentum = momentumRight;
                directionChanged = true;
            }
        }

        //apply buffered direction if possible
        if (canGoUp && m_buffer == momentumUp && m_momentum != momentumUp) {
            m_momentum = momentumUp;
            directionChanged = true;
        }
        else if (canGoDown && m_buffer == momentumDown && m_momentum != momentumDown) {
            m_momentum = momentumDown;
            directionChanged = true;
        }
        else if (canGoLeft && m_buffer == momentumLeft && m_momentum != momentumLeft) {
            m_momentum = momentumLeft;
            directionChanged = true;
        }
        else if (canGoRight && m_buffer == momentumRight && m_momentum != momentumRight) {
            m_momentum = momentumRight;
            directionChanged = true;
        }

        //snap to center only if direction changed
        if (directionChanged) {
            getSprite().setPosition({ centerX, centerY });
            pos = { centerX, centerY };
        }
    }

    //check if current tile is a corridor and close proximity to center
    else if (distanceToCenter <= epsilon || crossingCenter) {
        
        //keep track of direction change (only need to snap to center once)
        bool directionChanged = false;

        //check possible directions (with bounds checking)
        bool upBlocked = (row <= 0) || (_grid[row - 1][col] == '#');
        bool downBlocked = (row >= static_cast<int>(_grid.size() - 1)) || (_grid[row + 1][col] == '#');
        bool leftBlocked = (col <= 0) || (_grid[row][col - 1] == '#');
        bool rightBlocked = (col >= static_cast<int>(_grid[row].size() - 1)) || (_grid[row][col + 1] == '#');


        //set starting impulse if not moving yet
        if (m_momentum.x == 0.0f && m_momentum.y == 0.0f &&
            (m_buffer.x != 0.0f || m_buffer.y != 0.0f)) {
            if (!upBlocked && m_buffer == momentumUp) {
                m_momentum = momentumUp;
                directionChanged = true;
            }
            else if (!downBlocked && m_buffer == momentumDown) {
                m_momentum = momentumDown;
                directionChanged = true;
            }
            else if (!leftBlocked && m_buffer == momentumLeft) {
                m_momentum = momentumLeft;
                directionChanged = true;
            }
            else if (!rightBlocked && m_buffer == momentumRight) {
                m_momentum = momentumRight;
                directionChanged = true;
            }
        }

        //apply buffered movement if possible
        if ((m_momentum == momentumUp && m_buffer == momentumDown) ||
            (m_momentum == momentumDown && m_buffer == momentumUp) ||
            (m_momentum == momentumLeft && m_buffer == momentumRight) ||
            (m_momentum == momentumRight && m_buffer == momentumLeft)) {
            m_momentum = m_buffer;
            directionChanged = true;
        }

        //snap to center if direction changed
        if (directionChanged) {
            if (m_momentum.x != 0.0f) {
                getSprite().setPosition({ centerX, pos.y });
                pos.x = centerX;
            }
            if (m_momentum.y != 0.0f) {
                getSprite().setPosition({ pos.x, centerY });
                pos.y = centerY;
            }
        }
    }

    //clear buffer if direction changed
    if (m_momentum == m_buffer) {
        m_buffer = sf::Vector2f{ 0.0f, 0.0f };
    }

    //calculate new position
    sf::Vector2f newPos = {
        pos.x + m_momentum.x * m_speed,
        pos.y + m_momentum.y * m_speed
    };

    //check the sprite center position for collision detection
    int newCol = static_cast<int>(floor(newPos.x / _tileSize));
    int newRow = static_cast<int>(floor(newPos.y / _tileSize));

    //border checking and collision detection
    bool validMove = true;

    if (newRow < 0 || newRow >= static_cast<int>(_grid.size()) ||
        newCol < 0 || newCol >= static_cast<int>(_grid[newRow].size())) {
        validMove = false;
    }
    else if (_grid[newRow][newCol] == '#') {
        validMove = false;
    }
    //only updates player position if the move is valid
    if (validMove) {
        getSprite().setPosition(newPos);
    }
    else {
        //inverse momentum when hitting a wall
        m_momentum *= -1.0f; 

    }
}

//player input handler 
void Player::handleInput(sf::Keyboard::Key _key) {

    //update buffer with corresponding direction-vectors if a movement key is pressed
    if (_key == sf::Keyboard::Key::W) m_buffer = {0.0f, -1.0f};
    if (_key == sf::Keyboard::Key::S) m_buffer = {0.0f, 1.0f};
    if (_key == sf::Keyboard::Key::A) m_buffer = {-1.0f, 0.0f};
    if (_key == sf::Keyboard::Key::D) m_buffer = {1.0f, 0.0f};
}

//player input reciever (if ml-model dictates input) 
void Player::recieveInput(sf::Vector2f _buffer) {

    m_buffer = _buffer; 
}