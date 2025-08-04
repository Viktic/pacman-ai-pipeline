//
// Created by Viktor Brandmaier on 04.07.25.
//

#pragma once
#include "Entity.h"
#include <string>
#include <SFML/Window/Event.hpp>

class Player: public Entity {

    public:

        void handleInput(unsigned _borderX, unsigned _borderY);
        Player(const std::string& _textureFilePath, float _posX, float _posY);
        int getHealth();
        void setHealth(int _value);
        void move(float _rateX, float _rateY, unsigned _borderX, unsigned _borderY) override;

    private:
        int m_health;
        float m_speed;
};



