cmake_minimum_required(VERSION 3.2)
project(eterSecurity)

file(GLOB ETERSECURITY_HEADERS
    "*.h"
    "*.hpp"
)
file(GLOB ETERSECURITY_SOURCES
    "*.c"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${ETERSECURITY_HEADERS} ${ETERSECURITY_SOURCES})
