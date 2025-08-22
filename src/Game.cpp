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
    "#......##.E....#",
    "#.####.##.####.#",
    "#..............#",
    "#.##.#.##.#.##.#",
    "#....#.P..#....#",
    "#.##.#.##.#.##.#",
    "#..............#",
    "#.####.##.####.#",
    "#E.....##.....E#",
    "################"
}),
m_tileSize(80){
    m_window = sf::RenderWindow(sf::VideoMode({_windowSizeX, _windowSizeY}), _title);
    m_window.setFramerateLimit(144);
}



/*
  factory method to automatically create instances of the correct child-class and add them to
  the base class array in the Game class via polymorphism
*/
void Game::addEnemy(const std::string& _filePath, sf::Vector2u _spawnPosition) {
    Entity* pEntity = new Enemy(_filePath, _spawnPosition);
    pEntity->getSprite().setColor(sf::Color::Red);
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

    //loop through the grid array
    for (int i = 0; i < m_grid.size(); ++i) {
        for (int j = 0; j < m_grid[0].size(); ++j) {
            char curr  = m_grid[i][j];
            if (validCrossing(j, i)) {
                m_crossings.insert({j,i});
            }

            //TESTING ONLY (highlight crossing location in grid via hashing)
            /*
            if (m_crossings.find({j,i}) != m_crossings.end()) {
                float pX = j* m_tileSize + 0.5*m_tileSize;
                float pY = i* m_tileSize + 0.5*m_tileSize;
                addBorder({pX, pY}, m_tileSize, sf::Color::Red);
            }
            */

            //initialize game map
            switch (curr) {
                case 'P': {
                    unsigned px = j*m_tileSize + 0.5*m_tileSize;
                    unsigned py = i*m_tileSize + 0.5*m_tileSize;
                    Player::set({px, py});
                    break;
                }
                case 'E': {
                    unsigned ex = j*m_tileSize + 0.5*m_tileSize;
                    unsigned ey = i*m_tileSize + 0.5*m_tileSize;
                    addEnemy("/Users/viktorbrandmaier/Desktop/Studium Programmieren/OOP_Game/src/sprites/HannesSprite.png", {ex, ey});
                    break;
                }
                case '#': {
                    float bx = j*m_tileSize + 0.5*m_tileSize;
                    float by = i*m_tileSize + 0.5*m_tileSize;
                    addBorder({bx, by}, m_tileSize, sf::Color::Blue);
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
    Player* pPlayer = Player::get();

    while (m_window.isOpen()) {
        handleInput();
        for (size_t i = 0; i < m_pEntities.size(); ++i) {
            m_pEntities[i]->move(getTileSize(), getGrid(), m_crossings);
        }
        render();
    }
}

void Game::handleInput() {
    Player* pPlayer = Player::get();

    while (auto eventOpt = m_window.pollEvent()) { // eventOpt ist std::optional<sf::Event>
        if (eventOpt->is<sf::Event::Closed>()) {
            m_window.close();
        }
        else if (auto keyEvent = eventOpt->getIf<sf::Event::KeyPressed>()) {
            pPlayer->handleInput(keyEvent->code); // keyEvent->code ist sf::Keyboard::Key
        }
    }
}
const std::vector<std::string>& Game::getGrid() const {
    return m_grid;
}

sf::RenderWindow& Game::getWindow() {
    return m_window;
}

float Game::getTileSize() {
    return m_tileSize;
}

Game::~Game() {

    for (size_t i = 0; i < m_pEntities.size(); ++i) {
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











