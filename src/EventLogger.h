//
// Created by Viktor Brandmaier on 02.09.25.
// Modified for Unix/Linux compatibility
//

#pragma once
#include "LogData.h"
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include <unistd.h>
#include <sys/types.h>

class EventLogger {

public:

    EventLogger();
    int getSessionId();
    bool isSessionOpen();
    void initializeSession();
    void gatherLogData(LogData& _data);
    void writeLogData();
    sf::Vector2f forwardLogData(LogData& _data);
    void closeSession();
    ~EventLogger();

private:

    int m_stdinWrite;
    int m_stdoutRead;
    pid_t m_pythonPid;
    bool m_pythonProcessRunning;
    nlohmann::json m_session;
    std::fstream m_sessionStream;
    std::string m_rawDataDir;
    std::string m_rawDataManifest;
    std::string m_sessionPath;

};