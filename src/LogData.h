//
// Created by Viktor Brandmaier on 04.09.25.
//

#pragma once
#include <SFML/System/Vector2.hpp>
#include <vector>


//struct that contains all the datapoints for logging
struct LogData {
	sf::Vector2f m_playerScreenPosition; //
	std::vector<sf::Vector2f> m_enemyScreenPositions; //
	sf::Vector2i m_playerGridPosition; //
	std::vector<sf::Vector2i> m_enemyGridPositions;  //
	sf::Vector2f m_playerMomentum; //
	std::vector<sf::Vector2f> m_enemyMomenta; //
	sf::Vector2f m_playerBuffer; //
	int m_score; 
	unsigned long m_tick; 
};