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
Game::Game(unsigned _windowSizeX, unsigned _windowSizeY, const std::string& _title, bool _logGame) :
    m_borderX(_windowSizeX),
    m_borderY(_windowSizeY),
    m_gameRunning(true),
    m_gameInitialized(false),
    m_score(0),
    m_reward(0),
    m_logGame(_logGame),
    m_truncated(false),
    m_terminated(false),
    //ASCII-grid-map 
    m_grid(
{
    "###############",
    "#P.....#......#",
    "#.####...####.#",
    "#..E...#......#",
    "###############"
}),
m_tileSize(80){
    m_window = sf::RenderWindow(sf::VideoMode({_windowSizeX, _windowSizeY}), _title);
    m_window.setFramerateLimit(100);
    
    //EventLogger initialization
    m_pEventLogger = std::make_unique<EventLogger>();

}

//constructs a new enemy with unique_ptr ownership and places it in the m_pEntities vector
void Game::addEnemy(const std::string& _filePath, sf::Vector2f _spawnPosition) {

    // Load texture only once and cache it
    if (m_textureCache.find(_filePath) == m_textureCache.end()) {
        if (!m_textureCache[_filePath].loadFromFile(_filePath)) {
            std::cerr << "ERROR: Failed to load texture: " << _filePath << std::endl;
            return;
        }
    }
    m_pEntities.emplace_back(std::make_unique<Enemy>(m_textureCache[_filePath] ,_spawnPosition));
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

        //loads the pellet texture only once then caches it
        if (m_textureCache.find(_filePath) == m_textureCache.end()) {
        if (!m_textureCache[_filePath].loadFromFile(_filePath)) {
            std::cerr << "ERROR: Failed to load texture: " << _filePath << std::endl;
            return;
        }
    }

    m_pPellets.emplace_back(std::make_unique<Pellet>(m_textureCache[_filePath], _spawnPosition));
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
    
    //REWARD FUNCTION: reset the reward score
    m_reward = 0; 

    //initialize the logging session
    if (m_logGame == true) {
        m_pEventLogger->initializeSession();
    }

    //reset the frame count
    m_frameCount = 0; 

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

                //loads the player texture only once then caches it
                if (m_textureCache.find("sprites/HannesSprite.png") == m_textureCache.end()) {
                    if (!m_textureCache["sprites/HannesSprite.png"].loadFromFile("sprites/HannesSprite.png")) {
                        std::cerr << "ERROR: Failed to load player texture" << std::endl;
                        break;
                    }
                }

                auto player = std::make_unique<Player>(m_textureCache["sprites/HannesSprite.png"],sf::Vector2f(px, py));

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
    m_truncated = false; 
    m_terminated = false; 

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

        //REWARD FUNCTION: negative reward for coliding with enemy
        m_reward -= 10;

        m_terminated = true;

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

        //REWARD FUNCTION: positive reward for picking up a pellet
        m_reward += 1;

        //check if all pellets on the screen are cleared 
        if (m_score % m_pPellets.size() == 0 && m_score > 0) {
            resetPellets(m_pPellets);
            m_reward += 20;
        }
    }
}

//handles all inputs
void Game::handleInput() {

    Player* pPlayer = Player::instance;

    while (auto eventOpt = m_window.pollEvent()) {
        //close the window 
        if (eventOpt->is<sf::Event::Closed>()) {
            
            m_gameRunning = false; 
            m_truncated = true; 
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

                //resets step reward
                m_reward = 0.0f;

                //take a score stamp before the player-pellet collision is checked
                int scoreStamp = m_score;

                //get player buffer before input
                sf::Vector2f playerBufferStamp1 = pPlayer->getBuffer();
                //game input handler
                handleInput();
                //get player buffer after input 
                sf::Vector2f playerBufferStamp2 = pPlayer->getBuffer();

                LogData logData;
                logData.m_score = m_score;
                logData.m_tick = m_frameCount;
                logData.m_done = false;
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

                            logData.m_enemyScreenPositions.push_back(enemyPosition);
                            logData.m_enemyMomenta.push_back(pEnemy->getMomentum());
                            logData.m_enemyGridPositions.push_back(sf::Vector2i{ col, row });

                            //detect collisions between player and enemy
                            checkCollisionEnemy(*pPlayer, *pEnemy);


                        }
                    }
                    else if (pPlayer && m_pEntities[i].get() == pPlayer) {

                        //add the Player specific data to pLogData
                        sf::Vector2f playerPosition = pPlayer->getSprite().getPosition();
                        int col = static_cast<int>(floor(playerPosition.x / m_tileSize));
                        int row = static_cast<int>(floor(playerPosition.y / m_tileSize));
                        logData.m_playerScreenPosition = playerPosition;
                        logData.m_playerGridPosition = sf::Vector2i{ col, row };
                        logData.m_playerMomentum = pPlayer->getMomentum();
                        logData.m_playerBuffer = pPlayer->getBuffer();

                    }
                    m_pEntities[i]->move(getTileSize(), getGrid(), m_crossings);

                }

                //check collision between player and pellets
                for (size_t i = 0; i < m_pPellets.size(); ++i) {
                    if (m_pPellets[i]->getPickedUpState() == false) {
                        checkCollisionPellet(*(pPlayer), *(m_pPellets[i].get()));
                    }
                }

                //REWARD FUNCTION: decreases reward for time passing
                m_reward -= 0.01f;

                //base log interval every 10 frames 
                //log if buffer has changed 
                if ((m_frameCount % 10 == 0 || playerBufferStamp1 != playerBufferStamp2) && m_logGame == true) {
                    m_pEventLogger->gatherLogData(logData);
                }

                //forward the gamestate to the ml-model every 60 frames, if the session is over or if the player has collected a pellet
                if (true) {

                    //logs the current reward score
                    logData.m_reward = m_reward;

                    //log the truncated flag
                    logData.m_truncated = m_truncated;

                    //log the terminated flag
                    logData.m_done = m_terminated;

                    //log the active pellet positions
                    for (size_t i = 0; i < m_pPellets.size(); ++i) {
                        if (!m_pPellets[i]->getPickedUpState()) {
                            logData.m_pelletPositions.push_back(
                                m_pPellets[i]->getSprite().getPosition()
                            );
                        }
                    }

                    sf::Vector2f predictedBuffer = m_pEventLogger->forwardLogData(logData);


                    if (predictedBuffer == sf::Vector2f(-1.0f, -1.0f)) {
                        m_gameInitialized = false;
                        m_score = 0;
                        m_gameRunning = true;
                        //leaves the inner game-loop to trigger reset
                        break;
                    }

                    //DEBUGGING ONLY

                    pPlayer->recieveInput(predictedBuffer);
                }
    
                render();
                m_frameCount++; 
            }
        }

        //if GameOver wait for player input to restart the game or close the window
        else {
            //if GameOver push the gathered Data and close the current session stream
            if (m_logGame == true) {
                m_pEventLogger->writeLogData();
                m_pEventLogger->closeSession();
            }

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
    //clears the texture cache only at the end of the program
    m_textureCache.clear();
}











