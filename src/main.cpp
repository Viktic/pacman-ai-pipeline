#include "Player.h"
#include "Game.h"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <string>

int main()
{

    //All the relevant path configurations are stored in config/paths.json
    std::fstream file("config/paths.json"); 
    
    if (!file.is_open()) {
        std::cerr << "failed to open file: config/paths.json" << std::endl;
        return 1; 
    }

    //read the config file contents
    nlohmann::json j;
    file >> j; 
    file.close();

    //read the raw data directory from the config/paths.json file
    std::string rawDataDir = j["rawDataDir"];
    //read the raw data manifest 

    std::string rawDataDirManifest = j["rawDataDirManifest"];


    //read the raw data manifest from the back to retreive latest entry
    std::fstream manifest(rawDataDirManifest);

    if (!manifest.is_open()) {
        std::cerr << "failed to open file: data/raw/manifest.jsonl" << std::endl;
        return 1;
    }

   

    std::string line;
    std::string lastline;

    //loop through the manifest and save the last line
    while (std::getline(manifest, line)) {
        if (!line.empty())
            lastline = line;
    }

    
    if (lastline.empty()) {
        std::cerr << "error: file is empty" << std::endl; 
        return 1; 
    }

    //extract the session_id from the last manifest entry to determine the next session_id
    nlohmann::json last = nlohmann::json::parse(lastline); 
    int lastSessionId = last["session_id"];

    //DEBUGGING ONLY
    std::cout << "last session_id: " << lastSessionId << std::endl;




    //Initialize Game Instance
    Game* pGame = new Game(1600, 1000, "OOP-Game");

    pGame->run();


    delete pGame; 

    return 0;
}
