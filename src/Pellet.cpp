//
// Created by Viktor Brandmaier on 26.08.2025
//

#include "Pellet.h"


Pellet::Pellet(std::string& _textureFilePath, sf::Vector2f& _spawnPosition): 
m_texture(_textureFilePath), 
m_sprite(m_texture), 
m_pickedUp(false) {
	m_sprite.setPosition(_spawnPosition);
}

sf::Sprite& Pellet::getSprite() {
	return m_sprite; 
}

sf::Texture& Pellet::getTexture() {
	return m_texture; 
}