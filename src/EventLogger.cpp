//
// Created by Viktor Brandmaier on 02.09.25.
//

#include "EventLogger.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>


//EventLogger constructor
EventLogger::EventLogger() {

	//all the relevant path configurations are stored in config/paths.json
	std::fstream file("config/paths.json");

	//throw error if paths.json can't be opened 
	if (!file.is_open()) {
		std::cerr << "failed to open file: config/paths.json" << std::endl;
	}

	//read the config file contents
	nlohmann::json j;
	file >> j;
	file.close();

	//set the rawDataDir path
	m_rawDataDir = j["rawDataDir"];

	//set the rawDataManifest path
	m_rawDataManifest = j["rawDataDirManifest"]; 

}

//retrieves the latest sessions id from the manifest and increments it to get the current session_id
int EventLogger::getSessionId() {

	//open the manifest.jsonl 
	std::fstream manifest(m_rawDataManifest);
	if (!manifest.is_open()) {
		std::cerr << "failed to open file: data/raw/manifest.jsonl" << std::endl;
	}

	std::string line; 
	std::string lastLine; 

	//get the last json object in the manifest (lastLine) - brute-force approach for now
	while (std::getline(manifest, line)) {
		if (!line.empty()) {
			lastLine = line; 
		}
	}

	//throw error if manifest is empty
	if (lastLine.empty()) {
		std::cerr << "line is empty" << std::endl; 
	}

	//parse the lastLine json object and extract the last session_id
	nlohmann::json last = nlohmann::json::parse(lastLine); 
	int lastSessionId = last["session_id"];

	//increment the last sessoin_id to get the new session_id
	int newSessionId = ++lastSessionId;
	return newSessionId; 
}