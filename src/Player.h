//
// Created by Viktor Brandmaier on 03.07.25.
//

// #pragma once
#include <SFML/Graphics/Sprite.hpp>
#include "SFML/Graphics/Texture.hpp"


class Player {
public:
    Player(const char* _filePath);
    sf::Sprite& getSprite();
    void move(float _rateX, float _rateY);

private:
    sf::Texture m_texture;
    sf::Sprite m_sprite;
    int m_health;
    float m_positionX;
    float m_positionY;

};



