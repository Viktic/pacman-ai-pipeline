#include "Player.h"
#include "Game.h"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Sprite.hpp>

int main()
{
    //Initialize Game Instance
    Game* pGame = new Game(1600, 1000, "OOP-Game");

    pGame->run();


    delete pGame; 

    return 0;
}
