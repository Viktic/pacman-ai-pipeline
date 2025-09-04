#include "Player.h"
#include "Game.h"
#include "EventLogger.h"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <string>

int main()
{
    //initialize EventLogger
    EventLogger* pEventLogger = new EventLogger();
    pEventLogger->initializeSession();

    delete pEventLogger; 


    //initialize Game Instance
    Game* pGame = new Game(1600, 1000, "OOP-Game");

    pGame->run();


    delete pGame; 

    return 0;
}
