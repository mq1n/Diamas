cmake_minimum_required(VERSION 3.8)
project(NetEngine)

# Preprocessor flags
if (WIN32)
	add_definitions(-DWIN32)
	add_definitions(-D_WIN32)
endif()

add_definitions(-DNOMINMAX)

# Output folder
set(OUTPUT_DIR ${PROJECT_SOURCE_DIR}/../../../Bin/Utilities)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OUTPUT_DIR})

add_definitions(-DUSE_ASIO)
add_definitions(-D_WIN32_WINNT=0x0601)

file(GLOB NETENGINE_HEADERS
	"${PROJECT_SOURCE_DIR}/include/*.h"
)
file(GLOB NETENGINE_SOURCES
	"${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${NETENGINE_HEADERS} ${NETENGINE_SOURCES})

if(NETENGINE_BUILD_TEST)
	include_directories(include)
	add_executable(${PROJECT_NAME}Test ${NETENGINE_HEADERS} test/main.cpp test/NetClientManager.cpp test/NetServerManager.cpp test/NetPeerManager.cpp)
	target_link_libraries(${PROJECT_NAME}Test ${PROJECT_NAME})
endif()

#install(TARGETS ${PROJECT_NAME}
#		RUNTIME DESTINATION bin
#		LIBRARY DESTINATION lib
#		ARCHIVE DESTINATION lib
#)
#install(FILES ${NETENGINE_HEADERS} DESTINATION include)
