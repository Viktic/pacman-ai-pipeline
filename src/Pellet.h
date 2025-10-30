//
// Created by Viktor Brandmaier on 26.08.2025
//

#include <string>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#pragma once

class Pellet {

public:

    Pellet(const sf::Texture& _texture, sf::Vector2f& _spawnPosition);
	const sf::Texture& getTexture();
	const sf::Sprite& getSprite(); 
	bool getPickedUpState();
	void setPickedUpState(bool _state); 


private:

	bool m_pickedUp;
	const sf::Texture& m_texture;
	sf::Sprite m_sprite;
};