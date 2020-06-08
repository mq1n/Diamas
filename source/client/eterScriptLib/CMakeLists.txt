cmake_minimum_required(VERSION 3.2)
project(eterScriptLib)

file(GLOB ETERSCRIPTLIB_HEADERS
    "*.h"
    "*.hpp"
)
file(GLOB ETERSCRIPTLIB_SOURCES
    "*.c"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${ETERSCRIPTLIB_HEADERS} ${ETERSCRIPTLIB_SOURCES})
