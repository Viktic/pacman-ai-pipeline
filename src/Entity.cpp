//
// Created by Viktor Brandmaier on 03.07.25.
//

#include "Entity.h"
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include "Game.h"
#include <SFML/Graphics/Texture.hpp>


//Entitiy constructor
Entity::Entity(const sf::Texture& _texture, sf::Vector2f& _spawnPosition):

//in SMFL3 sprite needs to be initialized in the initializer list
    m_texture(_texture),
    m_sprite(m_texture)
{
    sf::Vector2u textureSize = m_texture.getSize();
    m_sprite.setOrigin({textureSize.x * 0.5f, textureSize.y * 0.5f});
    
    m_sprite.setScale({0.15f, 0.15f});
    m_sprite.setPosition({_spawnPosition});
}

//sprite getter
sf::Sprite& Entity::getSprite() {
    return m_sprite;
}

//texture getter
sf::Texture& Entity::getTexture() {
    return const_cast<sf::Texture&>(m_texture);
}




