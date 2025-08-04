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

    void run();
    void render();
    void handleInput();
    void addEntity(Type _entityType, const std::string& _filePath);
    sf::RenderWindow& getWindow();


private:
    std::vector<Entity*> m_pEntities;
    sf::RenderWindow m_window;
    unsigned m_borderX;
    unsigned m_borderY;

};



