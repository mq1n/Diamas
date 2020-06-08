cmake_minimum_required(VERSION 3.2)
project(eterGrnLib)

file(GLOB ETERGRNLIB_HEADERS
    "*.h"
    "*.hpp"
)
file(GLOB ETERGRNLIB_SOURCES
    "*.c"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${ETERGRNLIB_HEADERS} ${ETERGRNLIB_SOURCES})
