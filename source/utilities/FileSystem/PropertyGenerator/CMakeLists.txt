cmake_minimum_required(VERSION 3.8)
project(FSPropertyGenerator)
set(EXE_NAME PropertyGenerator)

# Preprocessor flags
if (WIN32)
	add_definitions(-DWIN32)
	add_definitions(-D_WIN32)
endif()

add_definitions(-DNOMINMAX)

# Project files
file(GLOB PROPERTYMGR_HEADERS
)
file(GLOB PROPERTYMGR_SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

set(EXTRA_LIBS dbghelp FSLib)
add_executable(${EXE_NAME}
    ${PROPERTYMGR_HEADERS}
    ${PROPERTYMGR_SOURCES}
)

target_link_libraries(${EXE_NAME} ${EXTRA_LIBS})

add_custom_command(
    TARGET ${EXE_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "LINKING: ${CMAKE_COMMAND} -E copy ${OUTPUT_DIR}/${EXE_NAME}.exe ${PROJECT_BINARY_DIR}/../../../../../../Diamas_Client/CRYPTION/${EXE_NAME}.exe"
    COMMAND ${CMAKE_COMMAND} -E copy ${OUTPUT_DIR}/${EXE_NAME}.exe ${PROJECT_BINARY_DIR}/../../../../../../Diamas_Client/CRYPTION/${EXE_NAME}.exe
)