//
// Created by Viktor Brandmaier on 02.09.25.
//

#pragma once
#include <string>
#include <fstream>

class EventLogger {

public: 

	EventLogger();
	int getSessionId(); 
	void initializeManifest(); 
	void initializeSession();

private: 

	std::fstream m_sessionStream; 
	std::string m_rawDataDir; 
	std::string m_rawDataManifest; 
	std::string m_sessionPath; 

};


