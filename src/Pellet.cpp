//
// Created by Viktor Brandmaier on 26.08.2025
//

#include "Pellet.h"
#include "Game.h"

Pellet::Pellet(const std::string& _textureFilePath, sf::Vector2f& _spawnPosition): 
m_texture(_textureFilePath), 
m_sprite(m_texture), 
m_pickedUp(false) {
	m_sprite.setPosition(_spawnPosition);
	m_sprite.setScale({ 0.05f, 0.05f});
}

const sf::Sprite& Pellet::getSprite() {
	return m_sprite; 
}

const sf::Texture& Pellet::getTexture() {
	return m_texture; 
}

bool Pellet::getPickedUpState() {
	return m_pickedUp; 
}