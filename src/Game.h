//
// Created by Viktor Brandmaier on 02.08.25.
//
#pragma once
#include "tool.h"
#include "EventLogger.h"
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_set>

class Pellet;
class Entity;
class Player;
class Enemy;

class Game {

public:

    Game(unsigned  _windowSizeX, unsigned  _windowSizeY, const std::string& _title, bool _logGame);
    ~Game();

    void initialize();
    void run();
    void render();
    void handleInput();
    void addEnemy(const std::string& _filePath, sf::Vector2f _spawnPosition);
    void addBorder(sf::Vector2f _spawnPosition, float _tileSize, sf::Color _color);
    void addPellet(const std::string& _filePath, sf::Vector2f _spawnPosition); 
    float getTileSize();
    void checkCollisionEnemy(Player& _player, Enemy& _enemy);
    void resetPellets(std::vector<std::unique_ptr<Pellet>>& _pellets);
    void checkCollisionPellet(Player& _player, Pellet& _pellet);
    bool getState();
    void clearGame(); 
    const std::vector<std::string>& getGrid() const;
    sf::RenderWindow& getWindow();


private:

    //private helper method to find crossings in the grid
    bool validCrossing(int _pX, int _pY);

    std::vector<std::unique_ptr<Pellet>> m_pPellets; 
    std::vector<std::string> m_grid;
    std::vector<std::unique_ptr<sf::RectangleShape>> m_pBorders;
    std::vector<std::unique_ptr<Entity>> m_pEntities; 
    sf::RenderWindow m_window;
    std::unique_ptr<EventLogger> m_pEventLogger; 

    //hash set which contains the grid coordinates of all valid crossings
    std::unordered_set<sf::Vector2i, tool::sfVector2iHash> m_crossings;

    float m_reward;
    int m_score; 
    float m_tileSize;
    unsigned m_borderX;
    unsigned m_borderY;
    bool m_logGame;
    bool m_gameRunning;
    bool m_gameInitialized;
    unsigned long m_frameCount; 
};



