cmake_minimum_required(VERSION 3.2)
project(sourceCommon)

file(GLOB SOURCECOMMON_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

add_library(${PROJECT_NAME} INTERFACE)
target_sources(${PROJECT_NAME} INTERFACE ${SOURCECOMMON_HEADERS})
