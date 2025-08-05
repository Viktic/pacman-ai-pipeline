//
// Created by Viktor Brandmaier on 04.07.25.
//

#pragma once
#include "Entity.h"
#include <string>
#include <SFML/Window/Event.hpp>

class Player: public Entity {

    public:
        static Player* get();

        Player(const Player&) = delete;
        Player& operator=(const Player&) = delete;

        void handleInput(sf::Vector2u _windowSize);
        int getHealth();
        void setHealth(int _value);
        void move(float _rateX, float _rateY, sf::Vector2u _windowSize) override;



    private:
        static Player* player;
        explicit Player(const std::string& _textureFilePath);



        int m_health;
        float m_speed;


};



