cmake_minimum_required(VERSION 3.2)
project(eterTerrainLib)

file(GLOB ETERTERRAINLIB_HEADERS
    "*.h"
    "*.hpp"
)
file(GLOB ETERTERRAINLIB_SOURCES
    "*.c"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${ETERTERRAINLIB_HEADERS} ${ETERTERRAINLIB_SOURCES})
