cmake_minimum_required(VERSION 3.21)
project(ReverseAndSpeedPortals VERSION 1.0.0)

add_library(${PROJECT_NAME} SHARED
    src/main.cpp
    src/ReversePortal.cpp
    src/CustomSpeedPortal.cpp
    src/PausePortal.cpp
)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Set the GEODE_SDK environment variable to the Geode SDK path (geode config).")
endif()

include($ENV{GEODE_SDK}/CMakeShared.cmake)

setup_geode_mod(${PROJECT_NAME})
