//
// Created by Viktor Brandmaier on 02.09.25.
//

#pragma once
#include <string>

class EventLogger {

public: 

	EventLogger();
	int getSessionId(); 
	void initializeManifest(); 

private: 

	std::string m_rawDataDir; 
	std::string m_rawDataManifest; 

};


