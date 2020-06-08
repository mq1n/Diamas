cmake_minimum_required(VERSION 3.2)
project(eterWebBrowser)

file(GLOB ETERWEBBROWSER_HEADERS
    "*.h"
    "*.hpp"
)
file(GLOB ETERWEBBROWSER_SOURCES
    "*.c"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${ETERWEBBROWSER_HEADERS} ${ETERWEBBROWSER_SOURCES})
