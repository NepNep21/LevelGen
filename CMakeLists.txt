cmake_minimum_required(VERSION 3.4)

project(LevelGen)

string(APPEND CMAKE_CXX_FLAGS "-Wall")

if(WIN32)
    string(APPEND CMAKE_CXX_FLAGS " -mwindows")
endif()

find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

add_executable(LevelGen LevelGen.cpp)

target_link_libraries(LevelGen Qt5::Core Qt5::Widgets)
