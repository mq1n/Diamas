cmake_minimum_required(VERSION 3.8)
project(FSUnpacker)
set(EXE_NAME Unpacker)

# Preprocessor flags
if (WIN32)
	add_definitions(-DWIN32)
	add_definitions(-D_WIN32)
endif()

add_definitions(-DNOMINMAX)

# Project files
file(GLOB UNPACKER_HEADERS
)
file(GLOB UNPACKER_SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

set(EXTRA_LIBS dbghelp FSLib)
add_executable(${EXE_NAME}
    ${UNPACKER_HEADERS}
    ${UNPACKER_SOURCES}
)

target_link_libraries(${EXE_NAME} ${EXTRA_LIBS})
