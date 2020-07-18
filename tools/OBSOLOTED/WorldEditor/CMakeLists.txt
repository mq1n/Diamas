cmake_minimum_required(VERSION 3.8)
project(WorldEditor)

add_definitions(-D_WIN32_WINNT=0x0501)
add_definitions(-DWINVER=0x0501)
add_definitions(-DWORLD_EDITOR)

include_directories(${PROJECT_SOURCE_DIR}/../../source/utilities/FileSystem/FSLib/include)

set(OUTPUT_DIR ${PROJECT_SOURCE_DIR}/../../Bin/Tools)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OUTPUT_DIR})

file(GLOB WORLDEDITOR_HEADERS
    "${PROJECT_SOURCE_DIR}/DataCtrl/*.h"
    "${PROJECT_SOURCE_DIR}/Dialog/*.h"
    "${PROJECT_SOURCE_DIR}/DockingBar/*.h"
    "${PROJECT_SOURCE_DIR}/Scene/*.h"
    "${PROJECT_SOURCE_DIR}/ToolBar/*.h"
    "${PROJECT_SOURCE_DIR}/UI/*.h"
    "${PROJECT_SOURCE_DIR}/*.h"
)
file(GLOB WORLDEDITOR_SOURCES
    "${PROJECT_SOURCE_DIR}/DataCtrl/*.cpp"
    "${PROJECT_SOURCE_DIR}/Dialog/*.cpp"
    "${PROJECT_SOURCE_DIR}/DockingBar/*.cpp"
    "${PROJECT_SOURCE_DIR}/Scene/*.cpp"
    "${PROJECT_SOURCE_DIR}/ToolBar/*.cpp"
    "${PROJECT_SOURCE_DIR}/UI/*.cpp"
    "${PROJECT_SOURCE_DIR}/*.cpp"
)
file(GLOB WORLDEDITOR_OTHER_INPUT
    "${PROJECT_SOURCE_DIR}/*.rc"
)
add_executable(${PROJECT_NAME} ${WORLDEDITOR_HEADERS} ${WORLDEDITOR_SOURCES} ${WORLDEDITOR_OTHER_INPUT})

target_precompile_headers(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/stdafx.h)

target_link_libraries(${PROJECT_NAME} FSLib sourceCommon eterBase eterEffectLib eterGameLib eterGrnLib eterImageLib eterLib eterScriptLib eterSoundLib eterSphereLib eterTerrainLib eterTreeLib)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY CMAKE_CXX_EXTENSIONS OFF)
