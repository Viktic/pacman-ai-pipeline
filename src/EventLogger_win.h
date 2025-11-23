//
// Created by Viktor Brandmaier on 23.11.25.
//

#pragma once
#include "LogData.h"
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include <cstdio>
#include <windows.h>

class EventLogger {

public: 

	EventLogger();
	int getSessionId();
	bool isSessionOpen(); 
	void initializeManifest(); 
	void initializeSession();
	void gatherLogData(LogData& _data); 
	void writeLogData();
	sf::Vector2f forwardLogData(LogData& _data); 
	void closeSession(); 
	~EventLogger();

private: 

	HANDLE m_hStdinWrite;
	HANDLE m_hStdoutRead;
	PROCESS_INFORMATION m_pi;
	bool m_pythonProcessRunning;
	nlohmann::json m_session; 
	std::fstream m_sessionStream; 
	std::string m_rawDataDir; 
	std::string m_rawDataManifest; 
	std::string m_sessionPath; 

};

