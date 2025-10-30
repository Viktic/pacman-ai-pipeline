//
// Created by Viktor Brandmaier on 04.07.25.
//

#pragma once

#include "Entity.h"
#include <string>
#include <SFML/Window/Event.hpp>

//the Player class is instanciated as a globally accessible unique instance
//lifetime of the player instance is managed externally (via unique_ptr) 
//Player class is derived from Entity class
class Player: public Entity {

    public:

        //static global access-point to the player instance 
        static Player* instance;

        Player(const sf::Texture& _texture,  sf::Vector2f _spawnPosition);
        void handleInput(sf::Keyboard::Key _key);
        void resetMomentum(); 
        void move(float _tileSize, const std::vector<std::string>& _grid, const std::unordered_set<sf::Vector2i, tool::sfVector2iHash>& _crossings) override;
        sf::Vector2f& getMomentum();
        sf::Vector2f& getBuffer(); 
        void recieveInput(sf::Vector2f _buffer);



    private:

        //Momentum Variable to keep track of the players movement direction
        sf::Vector2f m_momentum;

        //Momentum buffer Variable to keep track of the next direction
        sf::Vector2f m_buffer;

        float m_speed;

};



