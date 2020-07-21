cmake_minimum_required(VERSION 3.2)
project(auth_server)
set(EXE_NAME auth_server)

# Output dir
set(OUTPUT_DIR ${PROJECT_SOURCE_DIR}/../../../Bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OUTPUT_DIR})

# Preprocessor flags
if (MSVC)
	add_definitions(-DWIN32)
	add_definitions(-D_WIN32)
	add_definitions(-D__WIN32__)
endif()

add_definitions(-DNOMINMAX)
add_definitions(-DUSE_ASIO)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	add_definitions(-D_DEBUG)
	add_definitions(-DDEBUG)
	set(BUILD_MODE "debug")
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
	add_definitions(-D_NDEBUG)
	add_definitions(-DNDEBUG)
	set(BUILD_MODE "normal")
elseif(${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
	add_definitions(-D_NDEBUG)
	add_definitions(-DNDEBUG)
	set(BUILD_MODE "normal")
elseif(${CMAKE_BUILD_TYPE} STREQUAL "MinSizeRel")
	add_definitions(-D_NDEBUG)
	add_definitions(-DNDEBUG)
	set(BUILD_MODE "normal")
endif()

# Project files
file(GLOB AUTH_HEADERS
	"${PROJECT_SOURCE_DIR}/../common/*.h"
	"${PROJECT_SOURCE_DIR}/src/*.hpp"
)
file(GLOB AUTH_SOURCES
	"${PROJECT_SOURCE_DIR}/src/*.cpp"
)

if (MSVC)
	set(EXTRA_LIBS ws2_32 dbghelp shlwapi NetEngine)

	if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
		set(EXTRA_LIBS ${EXTRA_LIBS} libmariadb_debug mariadbclient_debug)
	else()
		set(EXTRA_LIBS ${EXTRA_LIBS} libmariadb mariadbclient)
	endif()

	add_executable(${EXE_NAME} ${AUTH_HEADERS} ${AUTH_SOURCES})
	
	target_link_libraries(${EXE_NAME} thecorelib ${EXTRA_LIBS} DevIL)

else()
	add_executable(${EXE_NAME} ${AUTH_HEADERS} ${AUTH_SOURCES})

	if(NOT MYSQL_LIBRARIES)
		FIND_LIBRARY(MYSQL_LIBRARIES libmysqlclient.a ${MYSQL_DIR}/lib NO_DEFAULT_PATH)
	endif()
#	FIND_LIBRARY(LIBZ libz.a /usr/lib)

	# Link
	target_link_libraries(${EXE_NAME} ${LIBZ} ssl crypto ${MYSQL_LIBRARIES} pthread thecorelib NetEngine)

endif()

if (WIN32)
    ADD_CUSTOM_COMMAND(
        TARGET ${EXE_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${OUTPUT_DIR}/${EXE_NAME}.exe ${PROJECT_BINARY_DIR}/../../../../share/bin/${EXE_NAME}.exe
    )
endif()