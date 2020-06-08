cmake_minimum_required(VERSION 3.2)
project(eterGameLib)

file(GLOB ETERGAMELIB_HEADERS
    "*.h"
    "*.hpp"
)
file(GLOB ETERGAMELIB_SOURCES
    "*.c"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${ETERGAMELIB_HEADERS} ${ETERGAMELIB_SOURCES})
