//
// Created by Viktor Brandmaier on 04.07.25.
//

#pragma once

#include "Entity.h"
#include <string>
#include <SFML/Window/Event.hpp>

class Player: public Entity {

    public:
        static Player* instance;
        //set method allows to initialize the player-singleton once with the spawnPosition
        Player(const std::string& _texturePath, sf::Vector2f _spawnPosition); 
  

        void handleInput(sf::Keyboard::Key _key);
        int getHealth();
        void resetMomentum(); 
        void setHealth(int _value);
        void move(float _tileSize, const std::vector<std::string>& _grid, const std::unordered_set<sf::Vector2i, tool::sfVector2iHash>& _crossings) override;
        sf::Vector2f& getMomentum();



    private:

        //Momentum Variable to keep track of the players movement direction
        sf::Vector2f m_momentum;
        //Momentum buffer Variable to keep track of the next direction
        sf::Vector2f m_buffer;

        int m_health;
        float m_speed;
};



