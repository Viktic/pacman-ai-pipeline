//
// Created by Viktor Brandmaier on 02.08.25.
//

#include "Game.h"
#include "Entity.h"
#include "Player.h"
#include "Enemy.h"
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>



Game::Game(unsigned _windowSizeX, unsigned _windowSizeY, const std::string& _title):
m_borderX(_windowSizeX),
m_borderY(_windowSizeY),
m_gameRunning(true),
m_gameInitialized(false),
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
        {0, -1},
        {0, 1},  
        {-1, 0}, 
        {1,0}   
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

void Game::clearGame() {
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


void Game::initialize() {

    clearGame(); 
    m_pEntities.clear(); 
    m_pBorders.clear();  

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
                    addEnemy("sprites/HannesSprite.png", {ex, ey});
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
        m_gameInitialized = true;
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


//check collision between player and enemies 
void Game::checkCollision(Player& _player, Enemy& _enemy) {

    sf::FloatRect playerBounds = _player.getSprite().getGlobalBounds();
    sf::FloatRect enemyBounds = _enemy.getSprite().getGlobalBounds();
    

    //hitbox tolerance relative to player sprite size

    sf::Vector2u playerSpriteSize = _player.getSprite().getTexture().getSize();
    sf::Vector2f playerSpriteScale = _player.getSprite().getScale(); 

    float playerToleranceX = playerBounds.size.x * 0.15f; 
    float playerToleranceY = playerBounds.size.y * 0.15f; 


    //hitbox tolerance relative to enemy sprite size

    sf::Vector2u enemySpriteSize = _enemy.getSprite().getTexture().getSize();
    sf::Vector2f enemySpriteScale = _enemy.getSprite().getScale();

    float enemyToleranceX = enemyBounds.size.x * 0.15f; 
    float enemyToleranceY = enemyBounds.size.y * 0.15f; 


    playerBounds.position.x += playerToleranceX;
    playerBounds.position.y += playerToleranceY;
    playerBounds.size.x -= 2 * playerToleranceX;
    playerBounds.size.y -= 2 * playerToleranceY;

    enemyBounds.position.x += enemyToleranceX;
    enemyBounds.position.y += enemyToleranceY;
    enemyBounds.size.x -= 2 * enemyToleranceX;
    enemyBounds.size.y -= 2 * enemyToleranceY;


    if (playerBounds.findIntersection(enemyBounds)) {
        m_gameRunning = false; 
        _player.resetMomentum();
    }
}

void Game::run() {

    while (m_window.isOpen()) { //outer game loop (handles the logic for creating a new game)
        if (!m_gameInitialized) {
            initialize();
        }


        if (m_gameRunning) {
            Player* pPlayer = Player::get();

            while (m_gameRunning) { //inner game loop (handles the current game logic)
                handleInput();
                for (size_t i = 0; i < m_pEntities.size(); ++i) {
                    m_pEntities[i]->move(getTileSize(), getGrid(), m_crossings);
                    //skip player to avoid self referring collision detection
                    if (m_pEntities[i] != pPlayer) {
                        //check pEnemy pointer after casting to avoid unexpected behaviour in case of unsuccessful cast
                        Enemy* pEnemy = dynamic_cast<Enemy*>(m_pEntities[i]);
                        if (pEnemy) {
                            checkCollision(*pPlayer, *pEnemy);
                        }
                    }
                }
                render();
            }
        }
        else {
            while (!m_gameRunning && m_window.isOpen()) { //if GameOver wait for player input to restart the game or close the window
                while (auto eventOpt = m_window.pollEvent()) {
                    if (eventOpt->is<sf::Event::Closed>()) {
                        m_window.close();
                    }
                    else if (auto keyEvent = eventOpt->getIf<sf::Event::KeyPressed>()) {
                        if (keyEvent->code == sf::Keyboard::Key::Enter) {
                            m_gameRunning = true;
                            m_gameInitialized = false;
                        }
                    }
                }
            }
        }
    }
}

void Game::handleInput() {
    Player* pPlayer = Player::get();

    while (auto eventOpt = m_window.pollEvent()) { // eventOpt is std::optional<sf::Event>
        if (eventOpt->is<sf::Event::Closed>()) {
            m_window.close();

        }

        else if (auto keyEvent = eventOpt->getIf<sf::Event::KeyPressed>()) {
            pPlayer->handleInput(keyEvent->code); // keyEvent->code is sf::Keyboard::Key
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

bool Game::getState() {
    return m_gameRunning;
}


Game::~Game() {
    clearGame();
    m_pEntities.clear();
    m_pBorders.clear();
}











