cmake_minimum_required(VERSION 3.2)
project(eterPythonLib)

file(GLOB ETERPYTHONLIB_HEADERS
    "*.h"
    "*.hpp"
)
file(GLOB ETERPYTHONLIB_SOURCES
    "*.c"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${ETERPYTHONLIB_HEADERS} ${ETERPYTHONLIB_SOURCES})
