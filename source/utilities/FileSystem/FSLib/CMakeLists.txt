cmake_minimum_required(VERSION 3.8)
project(FSLibrary)
set(EXE_NAME FSLib)

# Preprocessor flags
if (WIN32)
	add_definitions(-DWIN32)
	add_definitions(-D_WIN32)
endif()

add_definitions(-DNOMINMAX)

set(WARNINGS_DISABLE
	# ignore
	4244
)

foreach(d ${WARNINGS_DISABLE})
	set(WARNINGS "${WARNINGS} /wd${d}")
endforeach(d)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNINGS} ${MT_BUILD}")

set(CMAKE_CXX_STANDARD 17)
include_directories(${PROJECT_SOURCE_DIR}/../../../../extern/include/)

# Project files
file(GLOB LIB_HEADERS
	"${PROJECT_SOURCE_DIR}/include/*.hpp"
)
file(GLOB LIB_SOURCES
	"${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_library(${EXE_NAME}
	STATIC
	${LIB_HEADERS}
	${LIB_SOURCES}
)

if (ENABLE_STATIC_ANALYSIS)
	pvs_studio_add_target(
		TARGET fs_lib.analyze ALL
		OUTPUT FORMAT errorfile
		ANALYZE ${EXE_NAME}
		LOG fs_lib_analysis.err
	)
endif()
