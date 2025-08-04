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

    Player* pPlayer = dynamic_cast<Player*>(m_pEntities[0]);
    pPlayer->handleInput(m_borderX, m_borderY);
}



/*
  factory method to automatically create Instances of the right childClass and add them to
  the base class array in the Game class via polymorphism
*/
void Game::addEntity(Type _entityType, const std::string& _filePath, float _posX, float _posY) {
    Entity* pEntity = nullptr;
    switch (_entityType) {
        case Type::Player:
            pEntity =  new Player(_filePath, _posX, _posY);
            break;
        case Type::Enemy:
            pEntity = new Enemy(_filePath);
            break;
    }
    if (pEntity != nullptr) {
        m_pEntities.push_back(pEntity);

    }
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











