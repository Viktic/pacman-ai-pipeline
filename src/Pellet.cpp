//
// Created by Viktor Brandmaier on 26.08.2025
//

#include "Pellet.h"
#include "Game.h"

// Pellet constructor - takes texture reference instead of loading it
Pellet::Pellet(const sf::Texture& _texture, sf::Vector2f& _spawnPosition):
	m_texture(_texture),
	m_sprite(m_texture),
	m_pickedUp(false)
{
	sf::FloatRect spriteBounds = getSprite().getGlobalBounds();
	m_sprite.setPosition(_spawnPosition);
	m_sprite.setOrigin({ spriteBounds.size.x * 0.5f, spriteBounds.size.y * 0.5f});
	m_sprite.setScale({ 0.05f, 0.05f});
}

//sprite getter
const sf::Sprite& Pellet::getSprite() {
	return m_sprite; 
}

//texture getter
const sf::Texture& Pellet::getTexture() {
	return m_texture; 
}

//pickedUpState getter
bool Pellet::getPickedUpState() {
	return m_pickedUp; 
}

//pickedUpState setter
void Pellet::setPickedUpState(bool _state) {
	m_pickedUp = _state;
}