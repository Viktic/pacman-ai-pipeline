//
// Created by Viktor Brandmaier on 02.08.25.
//
#pragma once
#include "EntityType.h"
#include <vector>
#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_set>

class Entity;

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


    std::vector<std::string>* getGrid();
    sf::RenderWindow& getWindow();


private:
    //private helper method to find crossings in the grid
    bool validCrossing(int _pX, int _pY);

    struct m_pairHash {
        //creates hash for std::pair
        std::size_t operator() (std::pair<int, int> _pair) {
            return _pair.first * 42 + _pair.second;
        }
    };

    std::vector<std::string> m_grid;
    std::vector<sf::RectangleShape*> m_pBorders;
    std::vector<Entity*> m_pEntities;
    sf::RenderWindow m_window;

    //hash set which contains the grid coordinates of all valid crossings
    std::unordered_set<std::pair<int,int>, m_pairHash> m_crossings;

    unsigned m_borderX;
    unsigned m_borderY;

};



