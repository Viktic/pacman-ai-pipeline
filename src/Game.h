//
// Created by Viktor Brandmaier on 02.08.25.
//
#pragma once
#include "EntityType.h"
#include "tool.h"
#include <vector>
#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_set>


class Entity;
class Player;
class Enemy;

class Game {

public:
    Game(unsigned  _windowSizeX, unsigned  _windowSizeY, const std::string& _title);
    ~Game();

    void initialize();
    void run();
    void render();
    void handleInput();
    void addEnemy(const std::string& _filePath, sf::Vector2u _spawnPosition);
    void addBorder(sf::Vector2f _spawnPosition, float _tileSize, sf::Color _color);
    float getTileSize();
    void checkCollision(Player& _player, Enemy& _enemy);

    const std::vector<std::string>& getGrid() const;
    sf::RenderWindow& getWindow();


private:
    //private helper method to find crossings in the grid
    bool validCrossing(int _pX, int _pY);

    std::vector<std::string> m_grid;
    std::vector<sf::RectangleShape*> m_pBorders;
    std::vector<Entity*> m_pEntities;
    sf::RenderWindow m_window;

    //hash set which contains the grid coordinates of all valid crossings
    std::unordered_set<sf::Vector2i, tool::sfVector2iHash> m_crossings;

    float m_tileSize;
    unsigned m_borderX;
    unsigned m_borderY;
    bool m_gameRunning;

};



