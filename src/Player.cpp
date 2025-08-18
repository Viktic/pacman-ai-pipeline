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
m_speed(3.0f),
m_momentum({0,0}){

}

void Player::set(sf::Vector2u _spawnPosition) {
    if (player == nullptr) {
        player = new Player("/Users/viktorbrandmaier/Desktop/Studium Programmieren/OOP_Game/src/sprites/HannesSprite.png", _spawnPosition);
    }
}


Player* Player::get() {
    return player;
}


void Player::setHealth(int _value) {
    m_health = _value;
}

int Player::getHealth() {
    return m_health;
}

void Player::move(float _tileSize, std::vector<std::string>* _grid) {

    if (m_momentum == sf::Vector2i{0, 0}) return;

    sf::Vector2f scale = getSprite().getScale();
    //WORK IN PROGRESS
    sf::Vector2u size = getTexture().getSize();
    float offsetRight = size.x * scale.x;
    float offsetBottom = size.y * scale.y;


    sf::Vector2f pos = getSprite().getPosition();
    sf::Vector2f newPos = {
        pos.x + m_momentum.x * m_speed,
        pos.y + m_momentum.y * m_speed,
    };

    int col = std::round((newPos.x - 0.5f * _tileSize)/ _tileSize);
    int row = std::round((newPos.y - 0.5f * _tileSize) / _tileSize);

    if (row < 0 || row >= _grid->size() || col < 0 || col >= (*_grid)[0].size() || (*_grid)[row][col] == '#') {
        m_momentum = {0, 0}; //
        return;
    }

    getSprite().setPosition(newPos);
}

void Player::handleInput(sf::Keyboard::Key key) {
    if (key == sf::Keyboard::Key::W) m_momentum = {0, -1};
    if (key == sf::Keyboard::Key::S) m_momentum = {0, 1};
    if (key == sf::Keyboard::Key::A) m_momentum = {-1, 0};
    if (key == sf::Keyboard::Key::D) m_momentum = {1, 0};
}

