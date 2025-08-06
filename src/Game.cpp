//
// Created by Viktor Brandmaier on 02.08.25.
//

#include "Game.h"
#include "Entity.h"
#include "Player.h"
#include "Enemy.h"
#include <SFML/Window/Event.hpp>


Game::Game(unsigned _windowSizeX, unsigned _windowSizeY, const std::string& _title):
m_borderX(_windowSizeX),
m_borderY(_windowSizeY),
//every string represents a row in the grid
m_grid(//Player Position [1][1]
    {"################",
    "#......##......#",
    "#.####.##.####.#",
    "#.####.##.####.#",
    "#.####.##.####.#",
    "#.E....P...E...#",
    "################"}) {
    m_window = sf::RenderWindow(sf::VideoMode({_windowSizeX, _windowSizeY}), _title);
    m_window.setFramerateLimit(144);
}

/*
  factory method to automatically create instances of the correct child-class and add them to
  the base class array in the Game class via polymorphism
*/
void Game::addEnemy(const std::string& _filePath, sf::Vector2u _spawnPosition) {
    Entity* pEntity = new Enemy(_filePath, _spawnPosition);
    m_pEntities.push_back(pEntity);
}

void Game::initialize() {
    float tileSize = 100;

    //loop through the grid array
    for (int i = 0; i < m_grid.size(); ++i) {
        for (int j = 0; j < m_grid[0].size(); ++j) {
            char curr  = m_grid[i][j];
            switch (curr) {
                case 'P': {
                    unsigned px = j*tileSize + 0.5*tileSize;
                    unsigned py = i*tileSize + 0.5*tileSize;
                    Player::set({px, py});
                    break;
                }
                case 'E': {
                    unsigned ex = j*tileSize + 0.5*tileSize;
                    unsigned ey = i*tileSize + 0.5*tileSize;
                    addEnemy("/Users/viktorbrandmaier/Desktop/Studium Programmieren/OOP_Game/src/sprites/HannesSprite.png", {ex, ey});
                    break;
                }
                    //WORK IN  PROGRESS
                //add Initialization for Borders, and blank spaces
            }
        }
    }


    //insert the player into the array
    Entity* pPlayer = Player::get();
    m_pEntities.push_back(pPlayer);
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


void Game::run() {
    //game-loop
    while (m_window.isOpen()) {
        handleInput();
        render();
    }
}

void Game::handleInput() {
    while (auto event = m_window.pollEvent())
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


std::vector<std::string>* Game::getGrid() {
    return &m_grid;
}


sf::RenderWindow& Game::getWindow() {
    return m_window;
}



Game::~Game() {

    size_t entityCount = Entity::getEntityCount();
    for (size_t i = 0; i < entityCount; ++i) {
        Entity* curr = m_pEntities[i];
        if (curr != Player::get()) {
            delete curr;
        }
    }
}











