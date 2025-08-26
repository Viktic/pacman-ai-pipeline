//
// Created by Viktor Brandmaier on 26.08.2025
//

#include <string>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#pragma once

class Pellet {

public:
	Pellet(std::string& _textureFilePath, sf::Vector2f& _spawnPosition);
	sf::Texture& getTexture();
	sf::Sprite& getSprite(); 



private:
	bool m_pickedUp;
	sf::Sprite m_sprite;
	sf::Texture m_texture;
};