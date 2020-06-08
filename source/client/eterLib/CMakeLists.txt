cmake_minimum_required(VERSION 3.2)
project(eterLib)

file(GLOB ETERLIB_HEADERS
    "*.h"
    "*.hpp"
)
file(GLOB ETERLIB_SOURCES
    "*.c"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${ETERLIB_HEADERS} ${ETERLIB_SOURCES})
