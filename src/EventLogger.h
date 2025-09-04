//
// Created by Viktor Brandmaier on 02.09.25.
//

#pragma once
#include "LogData.h"
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>


class EventLogger {

public: 

	EventLogger();
	int getSessionId();
	bool isSessionOpen(); 
	void initializeManifest(); 
	void initializeSession();
	void gatherLogData(LogData& _data); 
	void writeLogData();
	void closeSession(); 
	~EventLogger();

private: 

	static int tickCount;

	nlohmann::json m_session; 
	std::fstream m_sessionStream; 
	std::string m_rawDataDir; 
	std::string m_rawDataManifest; 
	std::string m_sessionPath; 

};


