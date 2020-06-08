cmake_minimum_required(VERSION 3.2)
project(eterImageLib)

file(GLOB ETERIMAGELIB_HEADERS
    "*.h"
    "*.hpp"
)
file(GLOB ETERIMAGELIB_SOURCES
    "*.c"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${ETERIMAGELIB_HEADERS} ${ETERIMAGELIB_SOURCES})
