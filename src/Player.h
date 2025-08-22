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

        void handleInput(sf::Keyboard::Key _key);
        int getHealth();
        void setHealth(int _value);
        void move(float _tileSize, const std::vector<std::string>& _grid, const std::unordered_set<sf::Vector2i, tool::sfVector2iHash>& _crossings) override;



    private:
        static Player* player;
        explicit Player(const std::string& _textureFilePath, sf::Vector2u _spawnPosition);

        //Momentum Variable to keep track of the players movement direction
        sf::Vector2f m_momentum;
        //Momentum buffer Variable to keep track of the next direction
        sf::Vector2f m_buffer;

        int m_health;
        float m_speed;
};



