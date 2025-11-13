//
// Created by Viktor Brandmaier on 02.09.25.
// Modified for Unix/Linux compatibility
//

#include "EventLogger.h"
#include "tool.h"
#include <cstddef>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <cstdio>
#include <nlohmann/json.hpp>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstring>
#include <poll.h>
#include <fcntl.h>


EventLogger::EventLogger() :
    m_stdinWrite(-1),
    m_stdoutRead(-1),
    m_pythonPid(-1),
    m_pythonProcessRunning(false)
{
    signal(SIGPIPE, SIG_IGN);

    std::fstream file("config/paths.json");

    if (!file.is_open()) {
        std::cerr << "failed to open file: config/paths.json" << std::endl;
        return;
    }

    nlohmann::json j;
    file >> j;
    file.close();

    m_rawDataDir = j["rawDataDir"];
    m_rawDataManifest = j["rawDataDirManifest"];
    std::string ml_workerPath = j["ml_workerPath"];
    std::string pythonPath = j["pythonPath"];

    int stdinPipe[2];
    int stdoutPipe[2];

    if (pipe(stdinPipe) == -1 || pipe(stdoutPipe) == -1) {
        std::cerr << "Failed to create pipes!" << std::endl;
        return;
    }

    m_pythonPid = fork();

    if (m_pythonPid == -1) {
        std::cerr << "Failed to fork process!" << std::endl;
        close(stdinPipe[0]);
        close(stdinPipe[1]);
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);
        return;
    }

    if (m_pythonPid == 0) {
        if (dup2(stdinPipe[0], STDIN_FILENO) == -1) {
            perror("dup2 stdin");
            exit(1);
        }

        if (dup2(stdoutPipe[1], STDOUT_FILENO) == -1) {
            perror("dup2 stdout");
            exit(1);
        }

        if (dup2(stdoutPipe[1], STDERR_FILENO) == -1) {
            perror("dup2 stderr");
            exit(1);
        }

        close(stdinPipe[0]);
        close(stdinPipe[1]);
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);

        const char *arg1 = "-u";

        char *args[] = {
            (char*)pythonPath.c_str(),
            (char*)arg1,
            (char*)ml_workerPath.c_str(),
            nullptr
        };

        execv(pythonPath.c_str(), args);

        perror("execv");
        exit(1);
    } else {
        close(stdinPipe[0]);
        close(stdoutPipe[1]);

        m_stdinWrite = stdinPipe[1];
        m_stdoutRead = stdoutPipe[0];

        m_pythonProcessRunning = true;
    }
}

int EventLogger::getSessionId() {

    if (!std::filesystem::exists(m_rawDataManifest)) {
        std::ofstream manifest(m_rawDataManifest);
        if (!manifest.is_open()) {
            std::cerr << "failed to open file: " << m_rawDataManifest << std::endl;
        }
        manifest.close();
        return 0;
    }

    std::ifstream manifest(m_rawDataManifest, std::ifstream::binary);
    if (!manifest.is_open()) {
        std::cerr << "failed to open file: " << m_rawDataManifest << std::endl;
    }

    manifest.seekg(0, std::ios_base::end);
    int streamPos = manifest.tellg();

    char c;
    std::string lastLine;

    bool singularJsonObject = true;

    for (--streamPos; streamPos >= 0; --streamPos) {
        manifest.seekg(streamPos);
        manifest.read(&c, 1);
        if (c == '\n') {
            singularJsonObject = false;
            break;
        }
    }

    getline(manifest, lastLine);

    if (singularJsonObject == true) {
        manifest.read(&c, 1);
        lastLine.insert(lastLine.begin(), c);
    }

    if (lastLine.empty()) {
        std::cerr << "manifest.jsonl is empty" << std::endl;
    }

    nlohmann::json last = nlohmann::json::parse(lastLine);
    int lastSessionId = last["session_id"];

    int newSessionId = ++lastSessionId;

    manifest.close();
    return newSessionId;
}

void EventLogger::initializeSession() {

    m_session.clear();
    if (m_sessionStream.is_open()) {
        m_sessionStream.close();
    }

    int sessionId = getSessionId();
    std::string sessionIdString = std::to_string(sessionId);
    std::string sessionString = "sessions/session_" + sessionIdString + ".json";

    m_sessionPath = m_rawDataDir + "/" + sessionString;

    m_sessionStream.open(m_sessionPath, std::ios::out);
    if (!m_sessionStream.is_open()) {
        std::cerr << "failed to create session";
        return;
    }

    m_session["ticks"] = nlohmann::json::array();

    std::ofstream manifest(m_rawDataManifest, std::ofstream::app);
    if (!manifest.is_open()) {
        std::cerr << "failed to open manifest" << std::endl;
    }

    std::string jsonObject = "\n{\"session_id\":" + sessionIdString + ", \"file_path\":\"" + sessionString + "\" }";

    manifest << jsonObject;
    manifest.close();
}

void EventLogger::gatherLogData(LogData& _data) {

    nlohmann::json tick = {
        {"tick", _data.m_tick},
        {"player_position_screen", {_data.m_playerScreenPosition.x, _data.m_playerScreenPosition.y}},
        {"player_position_grid", {_data.m_playerGridPosition.x, _data.m_playerGridPosition.y}},
        {"player_momentum", {_data.m_playerMomentum.x, _data.m_playerMomentum.y}},
        {"player_buffer", {_data.m_playerBuffer.x, _data.m_playerBuffer.y}},
        {"score", _data.m_score}
    };

    //checks if there are enemy specific vectors
    if (_data.m_enemyGridPositions.size() != 0 && _data.m_enemyMomenta.size() != 0 && _data.m_enemyScreenPositions.size() != 0) {
        tick["enemy_positions_screen"] = nlohmann::json::array();
        tick["enemy_positions_grid"] = nlohmann::json::array();
        tick["enemy_momenta"] = nlohmann::json::array();

        for (size_t i = 0; i < _data.m_enemyScreenPositions.size(); ++i) {
            tick["enemy_positions_screen"].push_back({ _data.m_enemyScreenPositions[i].x, _data.m_enemyScreenPositions[i].y });
        }
        for (size_t i = 0; i < _data.m_enemyGridPositions.size(); ++i) {
            tick["enemy_positions_grid"].push_back({ _data.m_enemyGridPositions[i].x, _data.m_enemyGridPositions[i].y });
        }
        for (size_t i = 0; i < _data.m_enemyMomenta.size(); ++i) {
            tick["enemy_momenta"].push_back({ _data.m_enemyMomenta[i].x, _data.m_enemyMomenta[i].y });
        }
    }

    tick["directions"] = nlohmann::json::array();
    for (size_t i = 0; i < _data.m_validDirections.size(); ++i) {
        tick["directions"].push_back(_data.m_validDirections[i]);
    }

    m_session["ticks"].push_back(tick);
}

void EventLogger::writeLogData() {
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

sf::Vector2f EventLogger::forwardLogData(LogData& _data) {

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


    //checks if there are enemy specific vectors
    if (!_data.m_enemyGridPositions.empty() && !_data.m_enemyMomenta.empty() && !_data.m_enemyScreenPositions.empty()) {
        snapshot["enemy_positions_screen"] = nlohmann::json::array();
        snapshot["enemy_positions_grid"] = nlohmann::json::array();
        snapshot["enemy_momenta"] = nlohmann::json::array();


        for (size_t i = 0; i < _data.m_enemyScreenPositions.size(); ++i) {
            snapshot["enemy_positions_screen"].push_back({ _data.m_enemyScreenPositions[i].x, _data.m_enemyScreenPositions[i].y });
        }
        for (size_t i = 0; i < _data.m_enemyGridPositions.size(); ++i) {
            snapshot["enemy_positions_grid"].push_back({ _data.m_enemyGridPositions[i].x, _data.m_enemyGridPositions[i].y });
        }
        for (size_t i = 0; i < _data.m_enemyMomenta.size(); ++i) {
            snapshot["enemy_momenta"].push_back({ _data.m_enemyMomenta[i].x, _data.m_enemyMomenta[i].y });
        }
    }

    snapshot["pellet_positions"] = nlohmann::json::array();
    for (size_t i = 0; i < _data.m_pelletPositions.size(); ++i) {
        snapshot["pellet_positions"].push_back( {_data.m_pelletPositions[i].x, _data.m_pelletPositions[i].y });
    }

    snapshot["directions"] = nlohmann::json::array();
    for (size_t i = 0; i < _data.m_validDirections.size(); ++i) {
        snapshot["directions"].push_back(_data.m_validDirections[i]); 
    }

    
    std::string jsonLine = snapshot.dump() + "\n";
    ssize_t written = write(m_stdinWrite, jsonLine.c_str(), jsonLine.size());
    if (written == -1) {
        std::cerr << "Failed to write to python process" << std::endl;
        return sf::Vector2f(0, 0);
    }

    std::string response;
    char ch;

    struct pollfd pfd;
    pfd.fd = m_stdoutRead;
    pfd.events = POLLIN;

    const int TIMEOUT_MS = 10000;

    while (true) {
        int pollResult = poll(&pfd, 1, TIMEOUT_MS);

        if (pollResult == -1) {
            std::cerr << "Poll error" << std::endl;
            break;
        } else if (pollResult == 0) {
            std::cerr << "Read timeout from python process" << std::endl;
            break;
        }

        ssize_t bytesRead = read(m_stdoutRead, &ch, 1);

        if (bytesRead <= 0) {
            break;
        }

        if (ch == '\n') {
            break;
        }

        response.push_back(ch);

        if (!response.empty() && response.back() == '\r') {
            response.pop_back();
        }
    }

    if (response.empty()) {
        return sf::Vector2f(0, 0);
    }

    sf::Vector2f hashedDirection = tool::translationMap[response];

    return hashedDirection;
}

EventLogger::~EventLogger() {
    closeSession();

    if (m_stdinWrite != -1) {
        close(m_stdinWrite);
    }
    if (m_stdoutRead != -1) {
        close(m_stdoutRead);
    }

    if (m_pythonProcessRunning && m_pythonPid > 0) {
        kill(m_pythonPid, SIGTERM);

        int status;
        pid_t result = waitpid(m_pythonPid, &status, WNOHANG);
        if (result == 0) {
            sleep(1);
            kill(m_pythonPid, SIGKILL);
            waitpid(m_pythonPid, &status, 0);
        }
    }
}