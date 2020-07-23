cmake_minimum_required(VERSION 3.2)
project(eterClient)

set(OUTPUT_DIR ${PROJECT_SOURCE_DIR}/../../../Bin)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OUTPUT_DIR})

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	set(EXE_NAME client_debug_${TARGET_ARCH_STR})
else()
	set(EXE_NAME client_release_${TARGET_ARCH_STR})
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	add_definitions(-D_DEBUG)
	add_definitions(-DDEBUG)
	add_definitions(-DUSE_LOD)
	add_definitions(-DDUNGEON_WORK)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
	add_definitions(-D_NDEBUG)
	add_definitions(-DNDEBUG)
	add_definitions(-D_DISTRIBUTE)
	add_definitions(-DUSE_LOD)
	add_definitions(-DDUNGEON_WORK)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
	add_definitions(-D_NDEBUG)
	add_definitions(-DNDEBUG)
	add_definitions(-D_DISTRIBUTE)
	add_definitions(-DUSE_LOD)
	add_definitions(-DDUNGEON_WORK)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "MinSizeRel")
	add_definitions(-D_NDEBUG)
	add_definitions(-DNDEBUG)
	add_definitions(-D_DISTRIBUTE)
	add_definitions(-DUSE_LOD)
	add_definitions(-DDUNGEON_WORK)
endif()

file(GLOB ETERCLIENT_HEADERS
    "${PROJECT_SOURCE_DIR}/*.h"
    "${PROJECT_SOURCE_DIR}/*.hpp"
)
file(GLOB ETERCLIENT_SOURCES
    "${PROJECT_SOURCE_DIR}/*.c"
    "${PROJECT_SOURCE_DIR}/*.cpp"
)
file(GLOB ETERCLIENT_OTHER_INPUT
    "${PROJECT_SOURCE_DIR}/*.rc"
)
add_executable(${EXE_NAME} WIN32 ${ETERCLIENT_HEADERS} ${ETERCLIENT_SOURCES} ${ETERCLIENT_OTHER_INPUT})

SET_TARGET_PROPERTIES(${EXE_NAME} PROPERTIES LINK_FLAGS    "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\" /SUBSYSTEM:WINDOWS")

if (ENABLE_CYTHON)
	set(CYTHON_LIB eterCythonLib)
endif()

target_link_libraries(
	${EXE_NAME}
	sourceCommon
	eterBase ${CYTHON_LIB} eterEffectLib eterGameLib eterGrnLib eterImageLib eterLib eterPythonLib
	eterScriptLib eterSoundLib eterSphereLib eterSecurity eterTerrainLib eterTreeLib eterWebBrowser
)

set_property(TARGET ${EXE_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${EXE_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${EXE_NAME} PROPERTY CMAKE_CXX_EXTENSIONS OFF)

if (ENABLE_STATIC_ANALYSIS)
	pvs_studio_add_target(
		TARGET client.analyze ALL
        OUTPUT FORMAT errorfile
        ANALYZE ${EXE_NAME} eterBase ${CYTHON_LIB} eterEffectLib eterGameLib eterGrnLib eterImageLib eterLib eterSecurity eterPythonLib eterScriptLib eterSoundLib eterSphereLib eterSecurity eterTerrainLib eterTreeLib eterWebBrowser
		LOG client_analysis.err
	)
endif()

if (NOT CI_BUILD)
	ADD_CUSTOM_COMMAND(TARGET ${EXE_NAME}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${OUTPUT_DIR}/${EXE_NAME}.exe ${PROJECT_BINARY_DIR}/../../../../../Diamas_client/${EXE_NAME}.exe
	)
endif()