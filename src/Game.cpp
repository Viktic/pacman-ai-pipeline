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
m_grid(
{
    "################",
    "#......##......#",
    "#.####.##.####.#",
    "#..............#",
    "#.##.#.##.#.##.#",
    "#....#.P..#....#",
    "#.##.#.##.#.##.#",
    "#..............#",
    "#.####.##.####.#",
    "#E.....##.....E#",
    "################"
}) {
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

void Game::addBorder(sf::Vector2f _spawnPosition, float _tileSize, sf::Color _color) {
    sf::RectangleShape* rect = new sf::RectangleShape({_tileSize, _tileSize});
    rect->setPosition(_spawnPosition);
    rect->setFillColor(sf::Color::Black);
    rect->setOutlineColor(_color);
    rect->setOutlineThickness(5.f);
    m_pBorders.push_back(rect);
}


bool Game::validCrossing(int _pX, int _pY) {
    //check if current tile itself is empty
    if (m_grid[_pY][_pX] == '#') {
        return false;
    }
    //set directions for surrounding tiles
    std::vector<std::pair<int, int>> directions = {
        {0, -1}, //top
        {0, 1},  //bottom
        {-1, 0}, //left
        {1,0}   //right
    };
    //find valid index of neighbouring tiles
    std::vector<std::pair<int, int>> neighbours = {};
    for (size_t k = 0; k < directions.size(); ++k) {
        int dx = directions[k].first;
        int dy = directions[k].second;

        int nx = _pX + dx;
        int ny = _pY + dy;

        if (ny >= 0 && ny < m_grid.size() && nx >= 0 && nx < m_grid[0].size()) {
            if (m_grid[ny][nx] != '#') {
                neighbours.push_back({nx, ny});
            }
        }
    }

    //if there are less than two neighbours -> no crossing possible
    if (neighbours.size() < 2) {
        return false;
    }

    //if there are excactly two neighbours -> check if they are orthogonal
    if (neighbours.size() == 2) {
        //get both neighbours
        std::pair<int, int> pair1 = neighbours[0];
        std::pair<int, int> pair2 = neighbours[1];

        //extract the directions
        int dx1 = pair1.first - _pX;
        int dy1 = pair1.second - _pY;
        int dx2 = pair2.first - _pX;
        int dy2 = pair2.second - _pY;

        //check if they are orthogonal
        if (dx1 == -dx2 && dy1 == -dy2) {
            return false;
        }
        return true;
    }
    //if there are more than two neighbours -> crossing
    return true;
}



void Game::initialize() {
    float tileSize = 80;

    //loop through the grid array
    for (int i = 0; i < m_grid.size(); ++i) {
        for (int j = 0; j < m_grid[0].size(); ++j) {
            char curr  = m_grid[i][j];
            if (validCrossing(j, i)) {
                m_crossings.insert({j,i});
            }
            //TESTING ONLY (highlight crossing location in grid via hashing)
            if (m_crossings.find({j,i}) != m_crossings.end()) {
                float pX = j* tileSize + 0.5*tileSize;
                float pY = i* tileSize + 0.5*tileSize;
                addBorder({pX, pY}, tileSize, sf::Color::Red);
            }
            //initialize game map
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
                case '#': {
                    float bx = j*tileSize + 0.5*tileSize;
                    float by = i*tileSize + 0.5*tileSize;
                    addBorder({bx, by}, tileSize, sf::Color::Blue);
                    break;
                }
                case '.': {
                    break;
                }
            }
        }
    }

    //insert the player into the array
    Entity* pPlayer = Player::get();
    m_pEntities.push_back(pPlayer);
}

void Game::render() {

    m_window.clear();

    //render border-objects
    size_t borderCount = m_pBorders.size();

    for (size_t i = 0; i < borderCount; ++i) {
        sf::RectangleShape* curr = m_pBorders[i];
        m_window.draw(*curr);
    }
    //render entity-objects
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
        render();
        handleInput();
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
    size_t borderCount = m_pBorders.size();
    for (size_t i = 0; i < borderCount; ++i) {
        sf::RectangleShape* curr = m_pBorders[i];
        delete curr;
    }
}











