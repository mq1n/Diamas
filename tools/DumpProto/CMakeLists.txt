cmake_minimum_required(VERSION 3.8)
project(DumpProto)

add_definitions(-DNOMINMAX)

set(OUTPUT_DIR ${PROJECT_SOURCE_DIR}/../../Bin/Tools)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OUTPUT_DIR})

file(GLOB HEADER_FILES
    "${PROJECT_SOURCE_DIR}/include/*.h"
    "${PROJECT_SOURCE_DIR}/../../source/common/singleton.h"
    "${PROJECT_SOURCE_DIR}/../../source/client/eterBase/tea.h"
    "${PROJECT_SOURCE_DIR}/../../source/client/eterBase/lzo.h"
    "${PROJECT_SOURCE_DIR}/../../source/server/db/src/CsvReader.h"
    "${PROJECT_SOURCE_DIR}/../../source/server/db/src/ProtoReader.h"
)
file(GLOB SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/../../source/client/eterBase/tea.cpp"
    "${PROJECT_SOURCE_DIR}/../../source/client/eterBase/lzo.cpp"
    "${PROJECT_SOURCE_DIR}/../../source/server/db/src/CsvReader.cpp"
    "${PROJECT_SOURCE_DIR}/../../source/server/db/src/ProtoReader.cpp"
)
file(GLOB OTHER_INPUT_FILES
    "*.rc"
)

add_executable (${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES} ${OTHER_INPUT_FILES})
target_link_libraries(${PROJECT_NAME} sourceCommon thecorelib)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY CMAKE_CXX_EXTENSIONS OFF)
