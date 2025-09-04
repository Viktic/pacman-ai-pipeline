//
// Created by Viktor Brandmaier on 02.09.25.
//

#include "EventLogger.h"
#include <fstream>
#include <filesystem>
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

	//initialize manifest.jsonl if manifest does not exist yet
	if (!std::filesystem::exists(m_rawDataManifest)) {
		//create first manifest entry
		std::ofstream manifest(m_rawDataManifest);
		std::string jsonObject = "{\"session_id\":1, \"file_path\":\"sessions/session_1.json\" }";
		manifest << jsonObject;
		manifest.close();

		//create first session.json
		m_sessionStream.open(m_rawDataDir + "/sessions/session_1.json", std::ios::out);
		if (!m_sessionStream.is_open()) {
			std::cerr << "failed to create session" << std::endl;
		}
	}
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

	//flag to catch edge case where manifest contains only a single json object
	bool singularJsonObject = true; 

	//moves the stream position to the last newline character (beginning of the last jsonl line) 
	for (--streamPos; streamPos >= 0; --streamPos) {
		manifest.seekg(streamPos);
		manifest.read(&c, 1); 
		if (c == '\n') {
			singularJsonObject = false; 
			break; 
		}
	}

	//reads the last line (last json object)
	getline(manifest, lastLine);
	
	//read the first character if the manifest contains only a single json object
	if (singularJsonObject == true) {
		manifest.read(&c, 1);
		lastLine.insert(lastLine.begin(), c);
	}


	//throw error if manifest is empty
	if (lastLine.empty()) {
		std::cerr << "manifest.jonsl is empty" << std::endl;
	}

	//parse the lastLine json object and extract the last session_id
	nlohmann::json last = nlohmann::json::parse(lastLine); 
	int lastSessionId = last["session_id"];

	//increment the last sessoin_id to get the new session_id
	int newSessionId = ++lastSessionId;

	manifest.close();
	return newSessionId; 
}

//initializes a new session json file in the raw/sessions directory
void EventLogger::initializeSession() {

	//check if the sessionStream is already open
	if (m_sessionStream.is_open()) {
		return;
	}

	int sessionId = getSessionId(); 
	std::string sessionIdString = std::to_string(sessionId);
	std::string sessionString = "sessions/session_" + sessionIdString + ".json";

	m_sessionPath = m_rawDataDir + "/" + sessionString;
	

	//create the session
	m_sessionStream.open(m_sessionPath, std::ios::app);
	if (!m_sessionStream.is_open()) {
		std::cerr << "failed to create session";
		return; 
	}

	//open manifest in append mode
	std::ofstream manifest(m_rawDataManifest, std::ofstream::app);
	if (!manifest.is_open()) {
		std::cerr << "failed to open manifest" << std::endl; 
	}
	

	//write the new session information into the manifest
	std::string jsonObject = "\n{\"session_id\":" + sessionIdString + ", \"file_path\":\"" + sessionString + "\" }";
	
	manifest << jsonObject; 
	manifest.close(); 
}