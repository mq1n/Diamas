cmake_minimum_required(VERSION 3.2)
project(eterTreeLib)

file(GLOB ETERTREELIB_HEADERS
    "*.h"
    "*.hpp"
)
file(GLOB ETERTREELIB_SOURCES
    "*.c"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${ETERTREELIB_HEADERS} ${ETERTREELIB_SOURCES})
