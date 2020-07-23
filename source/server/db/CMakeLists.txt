cmake_minimum_required(VERSION 3.2)
project(db_cache CXX)
set(EXE_NAME db_cache)

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

# add_definitions(-D_USE_32BIT_TIME_T)
add_definitions(-DNOMINMAX)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	add_definitions(-D_DEBUG)
	add_definitions(-DDEBUG)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
	add_definitions(-D_NDEBUG)
	add_definitions(-DNDEBUG)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
	add_definitions(-D_NDEBUG)
	add_definitions(-DNDEBUG)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "MinSizeRel")
	add_definitions(-D_NDEBUG)
	add_definitions(-DNDEBUG)
endif()

# Project files

file(GLOB DBCACHE_HEADERS
    "src/*.h"
    "src/*.hpp"
)
file(GLOB DBCACHE_SOURCES
    "src/*.c"
    "src/*.cpp"
)

if (MSVC)
	set(EXTRA_LIBS ws2_32 dbghelp shlwapi)

	if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
		set(EXTRA_LIBS ${EXTRA_LIBS} libmariadb_debug mariadbclient_debug)
	else()
		set(EXTRA_LIBS ${EXTRA_LIBS} libmariadb mariadbclient)
	endif()

	add_executable(${EXE_NAME} ${DBCACHE_HEADERS} ${DBCACHE_SOURCES})
	
	target_link_libraries(${EXE_NAME} sourceCommon thecorelib sqllib gamelib ${EXTRA_LIBS} DevIL)

else()
	add_executable(${EXE_NAME} ${DBCACHE_HEADERS} ${DBCACHE_SOURCES})

	# Link
	target_link_libraries(${EXE_NAME} sourceCommon mysqlclient -ldl -lm -lpthread -pthread thecorelib sqllib gamelib)

endif()

if (ENABLE_STATIC_ANALYSIS)
	pvs_studio_add_target(
		TARGET db_cache.analyze ALL
        OUTPUT FORMAT errorfile
        ANALYZE ${EXE_NAME}
		LOG db_cache_analysis.err
	)
endif()

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

if (NOT CI_BUILD)
    ADD_CUSTOM_COMMAND(
        TARGET ${EXE_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${OUTPUT_DIR}/${OUTPUT_NAME} ${PROJECT_BINARY_DIR}/../../../../share/bin/${OUTPUT_NAME}
    )
endif()