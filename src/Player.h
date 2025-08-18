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
        //set method allows to initialize the player-singleton once with the spawnPosition
        static void set(sf::Vector2u _spawnPosition);


        //deleted copy constructor and assignment operator
        Player(const Player&) = delete;
        Player& operator=(const Player&) = delete;

        void handleInput(sf::Keyboard::Key key);
        int getHealth();
        void setHealth(int _value);
        void move(float _tileSize, std::vector<std::string>*) override;



    private:
        static Player* player;
        explicit Player(const std::string& _textureFilePath, sf::Vector2u _spawnPosition);

        //Momentum Variable to keep track of the players movement direction
        sf::Vector2i m_momentum;

        int m_health;
        float m_speed;
};



