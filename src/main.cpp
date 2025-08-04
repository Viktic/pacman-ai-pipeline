#include "Player.h"
#include "Game.h"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Sprite.hpp>

int main()
{
    //Initialize Game Instance
    Game* pGame = new Game(1600, 1000, "OOP-Game");

    pGame->addEntity(Type::Player, "/Users/viktorbrandmaier/Desktop/Studium Programmieren/OOP_Game/src/sprites/HannesSprite.png");
    pGame->addEntity(Type::Enemy, "/Users/viktorbrandmaier/Desktop/Studium Programmieren/OOP_Game/src/sprites/HannesSprite.png");

    pGame->run();



    return 0;
}
