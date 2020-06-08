cmake_minimum_required(VERSION 3.2)
project(eterBase)

add_definitions(-DNOMINMAX)

file(GLOB ETERBASE_HEADERS
    "*.h"
    "*.hpp"
    "Poly/*.h"
)
file(GLOB ETERBASE_SOURCES
    "*.c"
    "*.cpp"
    "Poly/*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${ETERBASE_HEADERS} ${ETERBASE_SOURCES})
