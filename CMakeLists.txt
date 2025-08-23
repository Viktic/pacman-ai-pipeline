cmake_minimum_required(VERSION 3.28)
project(OOP_Game LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#global c++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.1
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(SFML)

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
    src/EntityType.h
    src/tool.h
)

target_compile_features(main PRIVATE cxx_std_17)
target_link_libraries(main PRIVATE SFML::Graphics)

#copy assets into runtime directory
file(COPY ${CMAKE_SOURCE_DIR}/src/sprites DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
