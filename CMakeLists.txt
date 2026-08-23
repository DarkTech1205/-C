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
# The include path is confirmed now, but several enum members and one
# builder method we guessed (EditorTab::Portals, EditorButtonColor::Blue,
# ValueMenu::builder()) turned out wrong per the compiler. Rather than
# keep guessing one wrong symbol at a time, print the ACTUAL contents of
# object-collab's small public headers into the configure log -- that's
# ground truth, not another guess. Delete this block once the real API
# is confirmed and the code above is using it directly.
set(OC_INCLUDE_DIR "${CMAKE_BINARY_DIR}/geode-deps/smjs.object-collab/include")
set(OC_HEADERS_TO_DUMP
    CustomObject.hpp
    ObjectAPI.hpp
    ObjectIDSwap.hpp
    CustomLevelData.hpp
    object_collab.hpp
)
foreach(hdr ${OC_HEADERS_TO_DUMP})
    set(hdr_path "${OC_INCLUDE_DIR}/${hdr}")
    if(EXISTS "${hdr_path}")
        file(READ "${hdr_path}" hdr_content)
        message(STATUS "=== BEGIN ${hdr} ===")
        message(STATUS "${hdr_content}")
        message(STATUS "=== END ${hdr} ===")
    else()
        message(STATUS "=== ${hdr} not found at ${hdr_path} ===")
    endif()
endforeach()
# --- End temporary diagnostic -------------------------------------------
