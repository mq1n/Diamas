cmake_minimum_required(VERSION 3.8)
project(FSTest)
set(EXE_NAME FSTest)

# Preprocessor flags
if (WIN32)
	add_definitions(-DWIN32)
	add_definitions(-D_WIN32)
endif()

add_definitions(-DNOMINMAX)

# Project files
file(GLOB TESTER_HEADERS
)
file(GLOB TESTER_SOURCES
	"${PROJECT_SOURCE_DIR}/src/*.cpp"
)

set(EXTRA_LIBS dbghelp FSLib)
add_executable(${EXE_NAME}
    ${TESTER_HEADERS}
    ${TESTER_SOURCES}
)

target_link_libraries(${EXE_NAME} ${EXTRA_LIBS})

