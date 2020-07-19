cmake_minimum_required(VERSION 3.2)
project(eterCythonLib)

include_directories(${PROJECT_SOURCE_DIR}/../../../extern/include/python27)

file(GLOB ETERCYTHONLIB_SOURCES
	${PROJECT_SOURCE_DIR}/../../../../Diamas_Client/CRYPTION/source/cyTemp/_dummy.cpp
	${PROJECT_SOURCE_DIR}/../../../../Diamas_Client/CRYPTION/source/cyTemp/*.cpp
)

set(PYTHON_CMD C:/Python27/python)

add_custom_target(clear_cython_cache COMMAND ${PYTHON_CMD} clear_cython_cache.py WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/../../../../Diamas_Client/CRYPTION)
add_custom_target(special_process COMMAND ${PYTHON_CMD} special_process.py WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/../../../../Diamas_Client/CRYPTION)
add_custom_target(root_process COMMAND ${PYTHON_CMD} conv/root_process.py cython WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/../../../../Diamas_Client/CRYPTION DEPENDS special_process)

add_library(${PROJECT_NAME} STATIC ${ETERCYTHONLIB_SOURCES})
add_dependencies(${PROJECT_NAME} clear_cython_cache root_process)
