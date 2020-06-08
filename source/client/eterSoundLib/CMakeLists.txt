cmake_minimum_required(VERSION 3.2)
project(eterSoundLib)

file(GLOB ETERSOUNDLIB_HEADERS
    "*.h"
    "*.hpp"
)
file(GLOB ETERSOUNDLIB_SOURCES
    "*.c"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${ETERSOUNDLIB_HEADERS} ${ETERSOUNDLIB_SOURCES})
