//
// Created by Viktor Brandmaier on 02.08.25.
//
#pragma once
#include "EntityType.h"
#include <vector>
#include <SFML/Graphics.hpp>
#include <string>


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
    void addBorder(sf::Vector2f _spawnPosition, float _tileSize);


    std::vector<std::string>* getGrid();
    sf::RenderWindow& getWindow();


private:
    std::vector<std::string> m_grid;
    std::vector<sf::RectangleShape*> m_pBorders;
    std::vector<Entity*> m_pEntities;
    sf::RenderWindow m_window;
    unsigned m_borderX;
    unsigned m_borderY;

};



