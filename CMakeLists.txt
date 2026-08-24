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

# --- Temporary diagnostic ---------------------------------------------
# GameObjectType::Modifier fixed the Solid-block collision problem but
# turned out to be GD's trigger category (invisible during actual play,
# confirmed by ObjectTraits.hpp's own doc comments) -- wrong for objects
# that need to be visible and touchable like these. Rather than guess a
# third GameObjectType value blind, find and print the real enum
# definition from the generated bindings themselves.
file(GLOB_RECURSE GOT_MATCHES "${CMAKE_BINARY_DIR}/bindings/bindings/*.hpp")
foreach(f ${GOT_MATCHES})
    file(STRINGS "${f}" GOT_LINES REGEX "GameObjectType")
    if(GOT_LINES)
        message(STATUS "=== GameObjectType references in ${f} ===")
        foreach(line ${GOT_LINES})
            message(STATUS "  ${line}")
        endforeach()
    endif()
endforeach()
# Also try the raw pre-codegen bindings source, in case the enum itself is
# declared there rather than in the generated output.
file(GLOB_RECURSE GOT_SRC_MATCHES "${CMAKE_BINARY_DIR}/_deps/bindings-src/bindings/*Enum*" "${CMAKE_BINARY_DIR}/_deps/bindings-src/bindings/**/GameObjectType*")
foreach(f ${GOT_SRC_MATCHES})
    message(STATUS "=== Found possible enum source: ${f} ===")
    file(READ "${f}" f_content)
    message(STATUS "${f_content}")
endforeach()
# --- End temporary diagnostic -------------------------------------------
