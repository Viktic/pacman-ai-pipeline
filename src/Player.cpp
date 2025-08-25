//
// Created by Viktor Brandmaier on 04.07.25.
//

#include "Player.h"
#include "Game.h"
#include <SFML/System/Vector2.hpp>
#include <iostream>

Player* Player::player = nullptr;

Player::Player(const std::string &_textureFilePath, sf::Vector2u _spawnPosition):
Entity(_textureFilePath, _spawnPosition),
m_health(100),
m_speed(2.0f),
m_momentum({0.0f,0.0f}),
m_buffer({0.0f, 0.0f}){

}

void Player::set(sf::Vector2u _spawnPosition) {
    if (player == nullptr) {
        player = new Player("sprites/HannesSprite.png", _spawnPosition);
    }
    else {
        float posX = _spawnPosition.x; 
        float posY = _spawnPosition.y;
        player->getSprite().setPosition({posX, posY});
    }
}


Player* Player::get() {
    return player;
}


void Player::setHealth(int _value) {
    m_health = _value;
}

void Player::resetMomentum() {
    m_momentum = { 0.0f, 0.0f };
}


int Player::getHealth() {
    return m_health;
}

sf::Vector2f& Player::getMomentum() {
    return m_momentum;
}
 

void Player::move(float _tileSize, const std::vector<std::string>& _grid, const std::unordered_set<sf::Vector2i, tool::sfVector2iHash>& _crossings) {

    //first Momentum impulse
    if (m_momentum == sf::Vector2f{0.0f, 0.0f}) {
        m_momentum = m_buffer;
        m_buffer = {0.0f, 0.0f};
    }

    //sprite offset to player-position
    sf::Vector2f scale = getSprite().getScale();
    sf::Vector2u size = getTexture().getSize();
    float offsetX = size.x * scale.x * 0.5f;
    float offsetY = size.y * scale.y * 0.5f;

    //actual centered player position
    sf::Vector2f pos = getSprite().getPosition();


    sf::Vector2f newPos = {
        pos.x + m_momentum.x * m_speed,
        pos.y + m_momentum.y * m_speed,
    };

    float borderX = newPos.x;
    float borderY = newPos.y;

    if (m_momentum.x > 0) borderX  += offsetX;
    if (m_momentum.x < 0) borderX -= offsetX;
    if (m_momentum.y > 0) borderY += offsetY;
    if (m_momentum.y < 0) borderY -= offsetY;

    //next player grid position
    int nextCol = std::round((pos.x + m_momentum.x * m_speed - 0.5f * _tileSize) / _tileSize);
    int nextRow = std::round((pos.y + m_momentum.y * m_speed - 0.5f * _tileSize) / _tileSize);

    //changing directions orthogonally is always possible
    if (m_buffer == m_momentum * -1.0f) {
        m_momentum = m_buffer;
        m_buffer = {0.0f, 0.0f};
    }

    //check if the next grid position is a crossing
    if (_crossings.find({nextCol, nextRow}) != _crossings.end()) {
        //check if player is at the center of the current crossing
        float centerX = nextCol * _tileSize + 0.5f * _tileSize;
        float centerY = nextRow * _tileSize + 0.5f * _tileSize;
        sf::Vector2f spriteCenter = newPos;
        float epsilon = 2.0f;
        if (std::abs(spriteCenter.x - centerX) < epsilon &&
            std::abs(spriteCenter.y - centerY) < epsilon) {
            //if player is at the center of the current crossing check for the possible movements

            //buffer is not empty
            if (m_buffer != sf::Vector2f(0.0f, 0.0f)){
                m_momentum = m_buffer;
                m_buffer = {0.0f, 0.0f};
            }
        }
    }


    if (nextRow < 0 || nextRow >= _grid.size() || nextCol < 0 || nextCol >= _grid[0].size() || _grid[nextRow][nextCol] == '#') {
        m_momentum *= -1.0f;
        return;
    }
    getSprite().setPosition(newPos);

}


void Player::handleInput(sf::Keyboard::Key _key) {
    if (_key == sf::Keyboard::Key::W) m_buffer = {0.0f, -1.0f};
    if (_key == sf::Keyboard::Key::S) m_buffer = {0.0f, 1.0f};
    if (_key == sf::Keyboard::Key::A) m_buffer = {-1.0f, 0.0f};
    if (_key == sf::Keyboard::Key::D) m_buffer = {1.0f, 0.0f};
}