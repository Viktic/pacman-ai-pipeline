//
// Created by Viktor Brandmaier on 04.09.25.
//

#pragma once
#include <SFML/System/Vector2.hpp>


//struct that contains all the datapoints for logging
struct LogData {

	sf::Vector2f m_playerScreenPosition; 
	sf::Vector2f m_enemyScreenPosition; 
	sf::Vector2i m_playerGridPosition; 
	sf::Vector2i m_enemyGridPosition; 
	bool m_pelletPickedUp; 
	sf::Vector2f m_playerMomentum;
	sf::Vector2f m_enemyMomentum; 
	sf::Vector2f m_playerBuffer;
	int m_score; 
};