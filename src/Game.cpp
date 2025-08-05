//
// Created by Viktor Brandmaier on 02.08.25.
//

#include "Game.h"
#include "Entity.h"
#include "Player.h"
#include "Enemy.h"
#include <SFML/Window/Event.hpp>


Game::Game(unsigned _windowSizeX, unsigned _windowSizeY, const std::string& _title):
m_pEntities(),
m_borderX(_windowSizeX),
m_borderY(_windowSizeY) {
    m_window = sf::RenderWindow(sf::VideoMode({_windowSizeX, _windowSizeY}), _title);
    m_window.setFramerateLimit(144);
}

/*
  factory method to automatically create instances of the correct child-class and add them to
  the base class array in the Game class via polymorphism
*/
void Game::addEnemy(const std::string& _filePath) {
    Entity* pEntity = new Enemy(_filePath, m_window.getSize());
    m_pEntities.push_back(pEntity);
}

void Game::initialize() {
    //insert the player into the array
    Entity* pPlayer = Player::get();
    m_pEntities.push_back(pPlayer);
    //insert arbitrary amount of enemys into the array
    addEnemy("/Users/viktorbrandmaier/Desktop/Studium Programmieren/OOP_Game/src/sprites/HannesSprite.png");
}


void Game::run() {
    //game-loop
    while (m_window.isOpen()) {
        handleInput();
        render();
    }
}

void Game::render() {
    //render
    m_window.clear();

    //render gameObjects
    size_t entityCount = Entity::getEntityCount();

    for (size_t i = 0; i < entityCount; ++i) {
        Entity* curr = m_pEntities[i];
        m_window.draw(curr->getSprite());
    }

    m_window.display();
}

void Game::handleInput() {
    while (const std::optional event = m_window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            m_window.close();
        }

    }
    //handle Inputs in Player class
    Player* pPlayer = Player::get();
    pPlayer->handleInput(m_window.getSize());
}



sf::RenderWindow& Game::getWindow() {
    return m_window;
}


Game::~Game() {

    size_t entityCount = Entity::getEntityCount();
    for (size_t i = 0; i < entityCount; ++i) {
        Entity* curr = m_pEntities[i];
        delete curr;
    }

}











