cmake_minimum_required(VERSION 3.2)
project(eterSphereLib)

file(GLOB ETERSPHERELIB_HEADERS
    "*.h"
    "*.hpp"
)
file(GLOB ETERSPHERELIB_SOURCES
    "*.c"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${ETERSPHERELIB_HEADERS} ${ETERSPHERELIB_SOURCES})
