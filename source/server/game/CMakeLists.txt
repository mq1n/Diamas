cmake_minimum_required(VERSION 3.2)
project(game_server C CXX)
set(EXE_NAME game_server)

# Output dir
set(OUTPUT_DIR ${PROJECT_SOURCE_DIR}/../../../Bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OUTPUT_DIR})

#Liblua
include_directories("../liblua/include")

# Preprocessor flags
if (WIN32)
	add_definitions(-DWIN32)
	add_definitions(-D_WIN32)
	add_definitions(-D__WIN32__)
endif()

# add_definitions(-D_USE_32BIT_TIME_T)
add_definitions(-DNOMINMAX)

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
file(GLOB GAMESERVER_HEADERS
	"${PROJECT_SOURCE_DIR}/../common/*.h"
	"${PROJECT_SOURCE_DIR}/src/*.h"
	"${PROJECT_SOURCE_DIR}/src/*.hpp"
)
file(GLOB GAMESERVER_SOURCES
	"${PROJECT_SOURCE_DIR}/src/*.c"
	"${PROJECT_SOURCE_DIR}/src/*.cpp"
)

if (MSVC)
	link_directories(${PROJECT_SOURCE_DIR}/../liblua/lib)

	set(EXTRA_LIBS ws2_32 dbghelp shlwapi)

	if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
		set(EXTRA_LIBS ${EXTRA_LIBS} libmariadb_debug mariadbclient_debug)
	else()
		set(EXTRA_LIBS ${EXTRA_LIBS} libmariadb mariadbclient)
	endif()

	add_executable(${EXE_NAME} ${GAMESERVER_HEADERS} ${GAMESERVER_SOURCES})
	
	target_link_libraries(${EXE_NAME} sourceCommon lualib thecorelib sqllib gamelib ${EXTRA_LIBS} DevIL)

else()
	add_executable(${EXE_NAME} ${GAMESERVER_HEADERS} ${GAMESERVER_SOURCES})

	set(IL_LIBRARIES IL png tiff mng lcms jpeg jbig)

	# Link
	target_link_libraries(${EXE_NAME} sourceCommon mysqlclient cryptopp ${IL_LIBRARIES} lualib thecorelib sqllib gamelib -ldl -lm -lpthread -pthread)

endif()

set_property(TARGET ${EXE_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${EXE_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${EXE_NAME} PROPERTY CMAKE_CXX_EXTENSIONS OFF)

SET_TARGET_PROPERTIES(${EXE_NAME} PROPERTIES COMPILE_DEFINITIONS "${_definitions}") 
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	SET_TARGET_PROPERTIES(${EXE_NAME} PROPERTIES LINK_SEARCH_END_STATIC 1) #STATIC LINKING for clang
	set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS)       # remove -Wl,-Bdynamic 
	set(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS) 
	set(CMAKE_SHARED_LIBRARY_C_FLAGS)         # remove -fPIC 
	set(CMAKE_SHARED_LIBRARY_CXX_FLAGS) 
	set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)    # remove -rdynamic 
	set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS) 
endif()

if (ENABLE_STATIC_ANALYSIS)
	pvs_studio_add_target(
		TARGET game_server.analyze ALL
        OUTPUT FORMAT errorfile
        ANALYZE ${EXE_NAME} gamelib sqllib thecorelib
		LOG game_server_analysis.err
	)
endif()

if (WIN32 AND NOT CI_BUILD)
    add_custom_command(
        TARGET ${EXE_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "LINKING: ${CMAKE_COMMAND} -E copy ${OUTPUT_DIR}/${EXE_NAME}.exe ${PROJECT_BINARY_DIR}/../../../../share/bin/${EXE_NAME}.exe"
        COMMAND ${CMAKE_COMMAND} -E copy ${OUTPUT_DIR}/${EXE_NAME}.exe ${PROJECT_BINARY_DIR}/../../../../share/bin/${EXE_NAME}.exe
        COMMAND ${CMAKE_COMMAND} -E echo "DIR: ${PROJECT_BINARY_DIR}/../../../../win_server/"
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../../../../win_server/
        COMMAND ${CMAKE_COMMAND} -E echo "SYMLINK-REMOVE: C:/Python27/python.exe ${PROJECT_BINARY_DIR}/../../../../win_server/m2panel.py kaldir ${BUILD_MODE}"
        COMMAND C:/Python27/python.exe ${PROJECT_BINARY_DIR}/../../../../win_server/m2panel.py kaldir ${BUILD_MODE}
        COMMAND ${CMAKE_COMMAND} -E echo "SYMLINK-SETUP: C:/Python27/python.exe ${PROJECT_BINARY_DIR}/../../../../win_server/m2panel.py kur ${BUILD_MODE} ${TARGET_ARCH_STR}"
        COMMAND C:/Python27/python.exe ${PROJECT_BINARY_DIR}/../../../../win_server/m2panel.py kur ${BUILD_MODE} ${TARGET_ARCH_STR}
    )
endif()