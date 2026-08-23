cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
project(ReverseAndSpeedPortals VERSION 1.2.0)

add_library(${PROJECT_NAME} SHARED
    src/main.cpp
    src/ReversePortal.cpp
    src/CustomSpeedPortal.cpp
    src/PausePortal.cpp
    src/PurpleOrb.cpp
    src/GravityShiftOrb.cpp
)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to a Geode installation")
else()
    message(STATUS "Found Geode SDK: $ENV{GEODE_SDK}")
    set(GEODE_SDK $ENV{GEODE_SDK})
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

setup_geode_mod(${PROJECT_NAME})
