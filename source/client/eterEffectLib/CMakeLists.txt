cmake_minimum_required(VERSION 3.2)
project(eterEffectLib)

file(GLOB ETEREFFECTLIB_HEADERS
    "*.h"
    "*.hpp"
)
file(GLOB ETEREFFECTLIB_SOURCES
    "*.c"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${ETEREFFECTLIB_HEADERS} ${ETEREFFECTLIB_SOURCES})
