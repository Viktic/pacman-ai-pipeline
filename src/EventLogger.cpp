//
// Created by Viktor Brandmaier on 02.09.25.
//

#include "EventLogger.h"
#include "tool.h"
#include <fstream>
#include <filesystem>
#include <iostream>
#include <cstdio>
#include <nlohmann/json.hpp>
#include <windows.h>


//EventLogger constructor
EventLogger::EventLogger() :

	m_hStdinWrite(nullptr),
	m_hStdoutRead(nullptr),
	m_pi({}),
	m_pythonProcessRunning(false)
{

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

	//set the ml_workerPath
	std::string ml_workerPath = j["ml_workerPath"];

	//start the python process

	HANDLE hStdinRead;
	HANDLE hStdoutWrite;

	SECURITY_ATTRIBUTES sa{};
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = nullptr;

	//stdout pipe
	CreatePipe(&m_hStdoutRead, &hStdoutWrite, &sa, 0);
	//stdin pipe
	CreatePipe(&hStdinRead, &m_hStdinWrite, &sa, 0);

	STARTUPINFOA si{};
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdInput = hStdinRead;
	si.hStdOutput = hStdoutWrite;
	si.hStdError = hStdoutWrite;

	std::string cmd = "C:/Users/vikto/Desktop/Pacman-Pipeline/pacman-ai-pipeline/pipeline_venv/Scripts/python.exe -u " + ml_workerPath;
	if (!CreateProcessA(nullptr, cmd.data(), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &m_pi)) {
		std::cerr << "Failed to start Python process!" << std::endl;
	}

	//closes unused pipe ends
	CloseHandle(hStdinRead);
	CloseHandle(hStdoutWrite);

	//set the python-process-running flag to true
	m_pythonProcessRunning = true;

}

//retrieves the latest sessions id from the manifest and increments it to get the current session_id
int EventLogger::getSessionId() {

	//initialize manifest.jsonl if manifest does not exist yet
	if (!std::filesystem::exists(m_rawDataManifest)) {
		//create the manifest
		std::ofstream manifest(m_rawDataManifest);
		if (!manifest.is_open()) {
			std::cerr << "failed to open file: data/raw/manifest.jsonl" << std::endl;
		}
		manifest.close();
		return 0;
	}


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


	m_session.clear(); 
	//check if the sessionStream is still open
	if (m_sessionStream.is_open()) {
		m_sessionStream.close(); 
	}

	int sessionId = getSessionId(); 
	std::string sessionIdString = std::to_string(sessionId);
	std::string sessionString = "sessions/session_" + sessionIdString + ".json";

	m_sessionPath = m_rawDataDir + "/" + sessionString;
	

	//create the session
	m_sessionStream.open(m_sessionPath, std::ios::out);
	if (!m_sessionStream.is_open()) {
		std::cerr << "failed to create session";
		return; 
	}

	//intialize session json object;
	m_session["ticks"] = nlohmann::json::array(); 


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

void EventLogger::gatherLogData(LogData& _data) {

	//create a tick json object that contains all the important data from the current tick
	nlohmann::json tick = {
		{"tick", _data.m_tick},
		{"player_position_screen", {_data.m_playerScreenPosition.x, _data.m_playerScreenPosition.y}},
		{"player_position_grid", {_data.m_playerGridPosition.x, _data.m_playerGridPosition.y}},
		{"player_momentum", {_data.m_playerMomentum.x, _data.m_playerMomentum.y}},
		{"player_buffer", {_data.m_playerBuffer.x, _data.m_playerBuffer.y}},
		{"score", _data.m_score}
	};

	for (size_t i = 0; i < _data.m_enemyScreenPositions.size(); ++i) {
		tick["enemy_positions_screen"].push_back({ _data.m_enemyScreenPositions[i].x, _data.m_enemyScreenPositions[i].y });
	}
	for (size_t i = 0; i < _data.m_enemyGridPositions.size(); ++i) {
		tick["enemy_positions_grid"].push_back({ _data.m_enemyGridPositions[i].x, _data.m_enemyGridPositions[i].y });
	}
	for (size_t i = 0; i < _data.m_enemyMomenta.size(); ++i) {
		tick["enemy_momenta"].push_back({ _data.m_enemyMomenta[i].x, _data.m_enemyMomenta[i].y });
	}


	//write the current tick data into the session object
	m_session["ticks"].push_back(tick);

}

void EventLogger::writeLogData() {
	//write the session object into the session.json file
	m_sessionStream << m_session.dump(4);
}

void EventLogger::closeSession() {
	m_sessionStream.close();
}

bool EventLogger::isSessionOpen() {
	if (m_sessionStream.is_open()) {
		return true; 
	} 
	return false; 
}

//forwards a data snapshot to the model 
sf::Vector2f EventLogger::forwardLogData(LogData& _data) {

	//builds json snapshot
	nlohmann::json snapshot = {
		{"tick", _data.m_tick},
		{"player_position_screen", {_data.m_playerScreenPosition.x, _data.m_playerScreenPosition.y}},
		{"player_position_grid", {_data.m_playerGridPosition.x, _data.m_playerGridPosition.y}},
		{"player_momentum", {_data.m_playerMomentum.x, _data.m_playerMomentum.y}},
		{"player_buffer", {_data.m_playerBuffer.x, _data.m_playerBuffer.y}},
		{"score", _data.m_score},
		{"reward", _data.m_reward},
		{"truncated", _data.m_truncated},
		{"done", _data.m_done}
	};

	for (size_t i = 0; i < _data.m_enemyScreenPositions.size(); ++i) {
		snapshot["enemy_positions_screen"].push_back({ _data.m_enemyScreenPositions[i].x, _data.m_enemyScreenPositions[i].y });
	}
	for (size_t i = 0; i < _data.m_enemyGridPositions.size(); ++i) {
		snapshot["enemy_positions_grid"].push_back({ _data.m_enemyGridPositions[i].x, _data.m_enemyGridPositions[i].y });
	}
	for (size_t i = 0; i < _data.m_enemyMomenta.size(); ++i) {
		snapshot["enemy_momenta"].push_back({ _data.m_enemyMomenta[i].x, _data.m_enemyMomenta[i].y });
	}

	//sends json + newline delimiter
	std::string jsonLine = snapshot.dump() + "\n";
	DWORD written;
	WriteFile(m_hStdinWrite, jsonLine.c_str(), (DWORD)jsonLine.size(), &written, nullptr);

	//reads the json-line
	std::string response;
	char ch;
	DWORD read;
	while (true) {
		if (!ReadFile(m_hStdoutRead, &ch, 1, &read, nullptr) || read == 0) break;
		if (ch == '\n') break;
		response.push_back(ch);

		//remove carriage return
		if (!response.empty() && response.back() == '\r') {
			response.pop_back();
		}
	}
	
	//convert the predicted direction into a valid direction vector
	sf::Vector2f hashedDirection = tool::translationMap[response];
	


	return hashedDirection;
}

EventLogger::~EventLogger() {
	closeSession();
	
	//process pipe cleanup
	CloseHandle(m_hStdinWrite);
	CloseHandle(m_hStdoutRead);
	CloseHandle(m_pi.hThread);
}