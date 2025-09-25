cmake_minimum_required(VERSION 3.28)
project(OOP_Game LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#global c++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
#SFML
FETCHCONTENT_DECLARE(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.1
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
)
FETCHCONTENT_MAKEAVAILABLE(SFML)

#nlohmann/json for data-storage
FETCHCONTENT_DECLARE(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
)
FETCHCONTENT_MAKEAVAILABLE(nlohmann_json)


add_executable(main
    src/main.cpp
    src/Entity.cpp
    src/Entity.h
    src/Player.cpp
    src/Player.h
    src/Enemy.cpp
    src/Enemy.h
    src/Game.cpp
    src/Game.h
    src/tool.h
    src/Pellet.cpp
    src/Pellet.h 
    src/EventLogger.h 
    src/EventLogger.cpp "src/tool.cpp")

target_compile_features(main PRIVATE cxx_std_17)
target_link_libraries(main PRIVATE SFML::Graphics)
target_link_libraries(main PRIVATE nlohmann_json::nlohmann_json)

#copy assets into runtime directory
file(COPY ${CMAKE_SOURCE_DIR}/src/sprites DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

#copy paths.json into runtime directory every time the file has been changed
add_custom_command(TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/config/paths.json
        $<TARGET_FILE_DIR:main>/config/paths.json
)