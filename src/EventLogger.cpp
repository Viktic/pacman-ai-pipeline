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
	std::ifstream manifest(m_rawDataManifest, std::ifstream::binary);
	if (!manifest.is_open()) {
		std::cerr << "failed to open file: data/raw/manifest.jsonl" << std::endl;
	}

	//sets the stream position to the end of the file
	manifest.seekg(0, std::ios_base::end);
	int streamPos = manifest.tellg();

	char c; 
	std::string lastLine; 

	//moves the stream position to the last newline character (beginning of the last jsonl line) 
	for (--streamPos; streamPos >= 0; --streamPos) {
		manifest.seekg(streamPos);
		manifest.read(&c, 1); 
		if (c == '\n') {
			break; 
		}
	}

	//reads the last line (last json object)
	getline(manifest, lastLine); 


	//throw error if manifest is empty
	if (lastLine.empty()) {
		std::cerr << "line is empty" << std::endl; 
	}

	//parse the lastLine json object and extract the last session_id
	nlohmann::json last = nlohmann::json::parse(lastLine); 
	int lastSessionId = last["session_id"];

	//increment the last sessoin_id to get the new session_id
	int newSessionId = ++lastSessionId;

	manifest.close();
	return newSessionId; 
}