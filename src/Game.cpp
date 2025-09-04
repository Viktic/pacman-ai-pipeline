//
// Created by Viktor Brandmaier on 02.08.25.
//

#include "Game.h"
#include "Entity.h"
#include "Player.h"
#include "Enemy.h"
#include "Pellet.h"
#include "LogData.h"
#include "EventLogger.h"
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>

//game constructor
Game::Game(unsigned _windowSizeX, unsigned _windowSizeY, const std::string& _title) :
    m_borderX(_windowSizeX),
    m_borderY(_windowSizeY),
    m_gameRunning(true),
    m_gameInitialized(false),
    m_score(0),

    //ASCII-grid-map 
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
    m_window.setFramerateLimit(100);
    
    //EventLogger initialization
    m_pEventLogger = std::make_unique<EventLogger>();

}

//constructs a new enemy with unique_ptr ownership and places it in the m_pEntities vector
void Game::addEnemy(const std::string& _filePath, sf::Vector2f _spawnPosition) {
    m_pEntities.emplace_back(std::make_unique<Enemy>(_filePath, _spawnPosition));
}

//constructs a new rectangle object
void Game::addBorder(sf::Vector2f _spawnPosition, float _tileSize, sf::Color _color) {

    std::unique_ptr<sf::RectangleShape> pBorder = std::make_unique<sf::RectangleShape>(sf::Vector2f{ m_tileSize, m_tileSize });

    pBorder->setPosition(_spawnPosition);
    pBorder->setFillColor(sf::Color::Black );
    pBorder->setOutlineColor(_color);
    pBorder->setOutlineThickness(5.0f);
    m_pBorders.push_back(std::move(pBorder));
}

//constructs a new pellet with unique_ptr ownership and places it in the m_pEntities vector
void Game::addPellet(const std::string& _filePath, sf::Vector2f _spawnPosition) {
    m_pPellets.emplace_back(std::make_unique<Pellet>(_filePath, _spawnPosition));
}

//parses the game-map to find valid crossings
bool Game::validCrossing(int _pX, int _pY) {

    //check if current tile itself is empty
    if (m_grid[_pY][_pX] == '#') {
        return false;
    }

    //set directions for surrounding tiles
    std::vector<sf::Vector2i> directions = {
        {0, -1},
        {0, 1},  
        {-1, 0}, 
        {1,0}   
    };

    std::vector<sf::Vector2i> neighbours = {};

    //find valid index of neighbouring tiles
    for (size_t k = 0; k < directions.size(); ++k) {

        //x and y directions
        int dx = directions[k].x;
        int dy = directions[k].y;

        //neighbouring tiles in x and y directions
        int nx = _pX + dx;
        int ny = _pY + dy;

        //adds tile coordinates to the neighbours vector if it is a valid crossing
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
        sf::Vector2i pair1 = neighbours[0];
        sf::Vector2i pair2 = neighbours[1];

        //extract the directions
        int dx1 = pair1.x - _pX;
        int dy1 = pair1.y - _pY;
        int dx2 = pair2.x - _pX;
        int dy2 = pair2.y - _pY;

        //check if directions are orthogonal
        if (dx1 == -dx2 && dy1 == -dy2) {
            return false;
        }
        return true;
    }
    //if there are more than two neighbours -> crossing
    return true;
}

//resets all the ownership vectors and deletes allocated object instances
void Game::clearGame() {
 

    //clear borders vector
    m_pBorders.clear();
    //clear entities and pellets vector (automatically frees memory controlled by smart pointers) 
    m_pEntities.clear();
    m_pPellets.clear();
}

//intializes the game
void Game::initialize() {

    //clear game first to avoid memory leaks 
    clearGame();
    
    //initialize the logging session
    m_pEventLogger->initializeSession();

    //loop through the grid array to initialize game-objects
    for (int i = 0; i < m_grid.size(); ++i) {
        for (int j = 0; j < m_grid[0].size(); ++j) {
            char curr = m_grid[i][j];
            if (validCrossing(j, i)) {
                m_crossings.insert({ j,i });
            }

            //TESTING ONLY (highlight crossing location in grid via hashing)
            /*
            if (m_crossings.find({j,i}) != m_crossings.end()) {
                float pX = j* m_tileSize + 0.5*m_tileSize;
                float pY = i* m_tileSize + 0.5*m_tileSize;
                addBorder({pX, pY}, m_tileSize, sf::Color::Red);
            }
            */

            //screen position relative to grid-position
            float px = j * m_tileSize + 0.5 * m_tileSize;
            float py = i * m_tileSize + 0.5 * m_tileSize;

            //initialize game map
            switch (curr) {

                //initialize player instance
            case 'P': {
                auto player = std::make_unique<Player>("sprites/HannesSprite.png", sf::Vector2f(px, py));

                //insert the player into the m_pEntities vector
                Player::instance = player.get();
                m_pEntities.push_back(std::move(player));
                break;
            }
                    //initialize enemy instances + underlying pellet instances
            case 'E': {
                addPellet("sprites/PelletSprite.png", { px, py });
                addEnemy("sprites/HannesSprite.png", { px, py });
                break;
            }
                    //initialize borders
            case '#': {
                addBorder({ px,py }, m_tileSize, sf::Color::Blue);
                break;
            }
                    //initialize pellet instances 
            case '.': {
                addPellet("sprites/PelletSprite.png", { px, py });
                break;
            }
            }
        }
    }
    m_gameInitialized = true;

}

//renders game-objects
void Game::render() {

    m_window.clear();

    //render pellet-objects only when pickedUpState == false
    size_t pelletCount = m_pPellets.size();

    for (size_t i = 0; i < pelletCount; ++i) {
        if (m_pPellets[i]->getPickedUpState() == false) {
            m_window.draw(m_pPellets[i]->getSprite());
        }
    }

    //render border-objects
    size_t borderCount = m_pBorders.size();

    for (size_t i = 0; i < borderCount; ++i) {
        m_window.draw(*(m_pBorders[i].get()));
    }

    //render entity-objects
    for (size_t i = 0; i < m_pEntities.size(); ++i) {
        m_window.draw(m_pEntities[i]->getSprite());
    }

    m_window.display();
}


//check collision between player and enemies 
void Game::checkCollisionEnemy(Player& _player, Enemy& _enemy) {

    //player and enemy bounds
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
    float enemyToleranceX = enemyBounds.size.x * 0.30f; 
    float enemyToleranceY = enemyBounds.size.y * 0.30f; 

    //update playerBounds with hitbox tolerance
    playerBounds.position.x += playerToleranceX;
    playerBounds.position.y += playerToleranceY;
    playerBounds.size.x -= 2 * playerToleranceX;
    playerBounds.size.y -= 2 * playerToleranceY;

    //update enemyBounds with hitbox tolerance
    enemyBounds.position.x += enemyToleranceX;
    enemyBounds.position.y += enemyToleranceY;
    enemyBounds.size.x -= 2 * enemyToleranceX;
    enemyBounds.size.y -= 2 * enemyToleranceY;

    //detect collision between player and enemy
    if (playerBounds.findIntersection(enemyBounds)) {
        m_gameRunning = false; 
        _player.resetMomentum();
    }
}

//reset the pickedUpState of all pellets to false
void Game::resetPellets(std::vector<std::unique_ptr<Pellet>>& _pellets) {

    for (size_t i = 0; i < _pellets.size(); ++i) {
        _pellets[i]->setPickedUpState(false);
    }
}

//check collision between player and pellet
void Game::checkCollisionPellet(Player& _player, Pellet& _pellet) {

    //player and pellet bounds
    sf::FloatRect playerBounds = _player.getSprite().getGlobalBounds();
    sf::FloatRect pelletBounds = _pellet.getSprite().getGlobalBounds();

    //hitbox tolerance relative to player sprite size
    sf::Vector2u playerSpriteSize = _player.getSprite().getTexture().getSize();
    sf::Vector2f playerSpriteScale = _player.getSprite().getScale();
    float playerToleranceX = playerBounds.size.x * 0.15f;
    float playerToleranceY = playerBounds.size.y * 0.15f;

    //update playerBounds with hitbox tolerance
    playerBounds.position.x += playerToleranceX;
    playerBounds.position.y += playerToleranceY;
    playerBounds.size.x -= 2 * playerToleranceX;
    playerBounds.size.y -= 2 * playerToleranceY;

    //detect collision between player and pellet
    if (playerBounds.findIntersection(pelletBounds)) {
        _pellet.setPickedUpState(true);
        m_score++; 
        //check if all pellets on the screen are cleared 
        if (m_score % m_pPellets.size() == 0 && m_score > 0) {
            resetPellets(m_pPellets); 
        }
    }
}

//handles all inputs
void Game::handleInput() {

    Player* pPlayer = Player::instance;

    while (auto eventOpt = m_window.pollEvent()) {
        //close the window 
        if (eventOpt->is<sf::Event::Closed>()) {
            m_window.close();

        }
        //redirect input to player input handler if a key is pressed
        else if (auto keyEvent = eventOpt->getIf<sf::Event::KeyPressed>()) {
            pPlayer->handleInput(keyEvent->code);
        }
    }
}

//runs the game
void Game::run() {

    //outer game loop (handles the logic for creating a new game)
    while (m_window.isOpen()) { 
        if (!m_gameInitialized) {
            initialize();
        }
        if (m_gameRunning) {
          
            Player* pPlayer = Player::instance; 

            //inner game loop (handles the current game logic)
            while (m_gameRunning) { 
                //game input handler
                handleInput();
                LogData* pLogData = new LogData; 
                pLogData->m_score = m_score;
                //updates the movement for all entities and checks collision between player and enemies
                for (size_t i = 0; i < m_pEntities.size(); ++i) {

                    //skip player to avoid self referring collision detection
                    if (m_pEntities[i].get() != pPlayer) {
                        
                        //check pEnemy pointer after casting to avoid unexpected behaviour in case of unsuccessful cast
                        Enemy* pEnemy = dynamic_cast<Enemy*>(m_pEntities[i].get());
                        
                        if (pEnemy) {

                            //add the Enemy specific data to pLogData
                            sf::Vector2f enemyPosition = pEnemy->getSprite().getPosition();
                            int col = int((enemyPosition.x - 0.5f * m_tileSize) / m_tileSize);
                            int row = int((enemyPosition.y - 0.5f * m_tileSize) / m_tileSize);

                            pLogData->m_enemyScreenPositions.push_back(enemyPosition);
                            pLogData->m_enemyMomenta.push_back(pEnemy->getMomentum());
                            pLogData->m_enemyGridPositions.push_back(sf::Vector2i{ col, row });

                            //detect collisions between player and enemy
                            checkCollisionEnemy(*pPlayer, *pEnemy);
                        }
                    }
                    else if (pPlayer && m_pEntities[i].get() == pPlayer) {

                        //add the Player specific data to pLogData
                        sf::Vector2f playerPosition = pPlayer->getSprite().getPosition();
                        int col = static_cast<int>(floor(playerPosition.x / m_tileSize));
                        int row = static_cast<int>(floor(playerPosition.y / m_tileSize));
                        pLogData->m_playerScreenPosition = playerPosition;
                        pLogData->m_playerGridPosition = sf::Vector2i{ col, row };
                        pLogData->m_playerMomentum = pPlayer->getMomentum(); 
                        pLogData->m_playerBuffer = pPlayer->getBuffer();
                        
                    }
                    
                    m_pEntities[i]->move(getTileSize(), getGrid(), m_crossings);

                }
                //check collision between player and pellets
                for (size_t i = 0; i < m_pPellets.size(); ++i) {

                    if (m_pPellets[i]->getPickedUpState() == false) {
                        checkCollisionPellet(*(pPlayer), *(m_pPellets[i].get()));
                    }

                }
                render();



                m_pEventLogger->gatherLogData(*pLogData); 
                delete pLogData; 
            }
        }

        //if GameOver wait for player input to restart the game or close the window
        else {
            //if GameOver push the gathered Data and close the current session stream
            m_pEventLogger->writeLogData();
            m_pEventLogger->closeSession(); 


            while (!m_gameRunning && m_window.isOpen()) {
                while (auto eventOpt = m_window.pollEvent()) {
                    //close the window
                    if (eventOpt->is<sf::Event::Closed>()) {
                        m_window.close();
                    }
                    //restart the game if Enter key is pressed
                    else if (auto keyEvent = eventOpt->getIf<sf::Event::KeyPressed>()) {
                        if (keyEvent->code == sf::Keyboard::Key::Enter) {
                            m_gameRunning = true;
                            m_gameInitialized = false;
                            std::cout << m_score << std::endl; 
                            m_score = 0;
                        }
                    }
                }
            }
        }
    }
}



//grid-map getter
const std::vector<std::string>& Game::getGrid() const {
    return m_grid;
}

//window getter
sf::RenderWindow& Game::getWindow() {
    return m_window;
}

//tileSize getter
float Game::getTileSize() {
    return m_tileSize;
}

//game-running state getter
bool Game::getState() {
    return m_gameRunning;
}

//game destructor
Game::~Game() {
    if (m_pEventLogger && m_pEventLogger->isSessionOpen()) {
        m_pEventLogger->writeLogData();
        m_pEventLogger->closeSession(); 
    }
    clearGame();
}











