//
// Created by Viktor Brandmaier on 02.09.25.
// Modified for Unix/Linux compatibility
//

#include "EventLogger_unix.h"
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
    //ignore SIGPIPE signal
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

    //arrays to store read and write ends 
    int stdinPipe[2];
    int stdoutPipe[2];

    //try to create pipes 
    if (pipe(stdinPipe) == -1 || pipe(stdoutPipe) == -1) {
        std::cerr << "Failed to create pipes!" << std::endl;
        return;
    }

    //forks process to create childprocess from which the python process will be started
    m_pythonPid = fork();

    //close pipe ends if fork() fails, to avoid ressource leak
    if (m_pythonPid == -1) {
        std::cerr << "Failed to fork process!" << std::endl;
        close(stdinPipe[0]);
        close(stdinPipe[1]);
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);
        return;
    }

    //child process (python process)
    if (m_pythonPid == 0) {
        //trys to redirect standard input of python process to stdinPipe[0]
        //dup2() increases reference counter to pipe object
        if (dup2(stdinPipe[0], STDIN_FILENO) == -1) {
            perror("dup2 stdin");
            exit(1);
        }
        //trys to redirect standard output of python process to stdoutPipe[1]
        //dup2() increases reference counter to pipe object
        if (dup2(stdoutPipe[1], STDOUT_FILENO) == -1) {
            perror("dup2 stdout");
            exit(1);
        }
        //trys to redirect standard error of python process to stdoutPipe[1]
        //dup2() increases reference counter to pipe object
        if (dup2(stdoutPipe[1], STDERR_FILENO) == -1) {
            perror("dup2 stderr");
            exit(1);
        }

        //close pipe ends that now have aliases to decrease reference counter
        close(stdinPipe[0]);
        close(stdoutPipe[1]);
        
        //close pipe ends that are only used for communication by parent process
        close(stdinPipe[1]);
        close(stdoutPipe[0]);

        const char *arg1 = "-u";

        char *args[] = {
            (char*)pythonPath.c_str(),
            (char*)arg1,
            (char*)ml_workerPath.c_str(),
            //signals the end of args string
            nullptr
        };

        //starts python process
        execv(pythonPath.c_str(), args);

        //this code can only be reached if execv fails
        perror("execv");
        exit(1);

    } else {

        //close pipe ends that are only used by python process
        close(stdinPipe[0]);
        close(stdoutPipe[1]);

        //save pipe ends for parent process communication
        m_stdinWrite = stdinPipe[1];
        m_stdoutRead = stdoutPipe[0];

        m_pythonProcessRunning = true;
    }
}

//retrieves the current session id from the manifest to calculate the new session id
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

//initializes a new session for event logging
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

//turn logData into JSON snapshot
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

//writes the logData into the session stream
void EventLogger::writeLogData() {
    m_sessionStream << m_session.dump(4);
}

//closes the session stream
void EventLogger::closeSession() {
    m_sessionStream.close();
}

//checks if session stream is opened
bool EventLogger::isSessionOpen() {
    if (m_sessionStream.is_open()) {
        return true;
    }
    return false;
}

//creates JSON snapshot and sends it to the python process
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

    snapshot["wall_distances"] = nlohmann::json::array(); 
    for (size_t i = 0; i < _data.m_wallDistances.size(); ++i) {
        snapshot["wall_distances"].push_back(_data.m_wallDistances[i]);
    }

    std::string jsonLine = snapshot.dump() + "\n";
    ssize_t written = write(m_stdinWrite, jsonLine.c_str(), jsonLine.size());
    if (written == -1) {
        std::cerr << "Failed to write to python process" << std::endl;
        return sf::Vector2f(0, 0);
    }

    std::string response;
    char ch;

    //instructs poll system to only monitor input events for read end in parent process
    struct pollfd pfd;
    pfd.fd = m_stdoutRead;
    pfd.events = POLLIN;

    const int TIMEOUT_MS = 10000;

    while (true) {
        //waits for poll event with 10s timeout window
        int pollResult = poll(&pfd, 1, TIMEOUT_MS);

        if (pollResult == -1) {
            std::cerr << "Poll error" << std::endl;
            break;
        } else if (pollResult == 0) {
            std::cerr << "Read timeout from python process" << std::endl;
            break;
        }

        size_t bytesRead = read(m_stdoutRead, &ch, 1);

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

    //closes open pipe ends
    if (m_stdinWrite != -1) {
        close(m_stdinWrite);
    }
    if (m_stdoutRead != -1) {
        close(m_stdoutRead);
    }

    if (m_pythonProcessRunning && m_pythonPid > 0) {
        //instructs the childprocess to end its work
        kill(m_pythonPid, SIGTERM);

        int status;
        //checks if the child process complied immediatly (WNOHANG)
        pid_t result = waitpid(m_pythonPid, &status, WNOHANG);
        if (result == 0) {
            //kills the child process if it did not comply
            sleep(1);
            kill(m_pythonPid, SIGKILL);
            //waits for exit code to avoid zombie process
            waitpid(m_pythonPid, &status, 0);
        }
    }
}