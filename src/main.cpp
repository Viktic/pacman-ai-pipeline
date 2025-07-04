#include "Player.h"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Sprite.hpp>

int main()
{
    //new window is declared and created
    auto window = sf::RenderWindow(sf::VideoMode({1920u, 1080u}), "OOP-Game");
    window.setFramerateLimit(144);

    auto pHannes = new Player("/Users/viktorbrandmaier/Desktop/Studium Programmieren/OOP_Game/src/sprites/HannesSprite.png");
    sf::Sprite& pHannesPlayer = pHannes->getSprite();
    //Game Loop
    while (window.isOpen())
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
            pHannes->move(1.5f, 0.0f);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
            pHannes->move(-1.5f, 0.0f);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
            pHannes->move(0.0f, 1.5f);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
            pHannes->move(0.0f, -1.5f);
        }
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                delete pHannes;
                window.close();
            }
        }
        //Render
        window.clear();

        //Draw the player Sprite
        window.draw(pHannesPlayer);

        window.display();
    }
}
