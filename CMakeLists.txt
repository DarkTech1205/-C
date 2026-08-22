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
# setup_geode_mod() is what triggers Geode's dependency resolution (the
# "Fetching dependency from index" lines in the build log), which is what
# populates ${CMAKE_BINARY_DIR}/geode-deps. Globbing it AFTER that call and
# printing what we find means the real path/filename for object-collab's
# headers shows up in the normal build log automatically -- no separate
# debug CI run needed to go find it by hand. Delete this block once the
# #include lines in the .hpp files are confirmed correct.
file(GLOB_RECURSE OBJECT_COLLAB_FILES "${CMAKE_BINARY_DIR}/geode-deps/*")
message(STATUS "=== geode-deps contents (for object-collab include path) ===")
foreach(f ${OBJECT_COLLAB_FILES})
    message(STATUS "  ${f}")
endforeach()
message(STATUS "=== end geode-deps contents ===")
# --- End temporary diagnostic -------------------------------------------
