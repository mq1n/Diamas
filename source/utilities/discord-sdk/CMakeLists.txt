cmake_minimum_required(VERSION 3.2)
project(discord_sdk)

# Output folder
set(OUTPUT_DIR ${PROJECT_SOURCE_DIR}/../../../Bin/Utilities)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OUTPUT_DIR})

file(GLOB DISCORDSDK_HEADERS
    "src/*.h"
    "src/*.hpp"
)
file(GLOB DISCORDSDK_SOURCES
    "src/*.c"
    "src/*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${DISCORDSDK_HEADERS} ${DISCORDSDK_SOURCES})
