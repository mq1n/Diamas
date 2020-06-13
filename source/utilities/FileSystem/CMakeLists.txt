cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(FileSystem)

# Windows headers require the default packing option. Changing this can lead to memory corruption
add_definitions(-DWINDOWS_IGNORE_PACKING_MISMATCH)

# Output folder
set(OUTPUT_DIR ${PROJECT_SOURCE_DIR}/../../../Bin/Utilities)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OUTPUT_DIR})

# General linker options
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO /NODEFAULTLIB:libci.lib")

# Sub projects
add_subdirectory(${PROJECT_SOURCE_DIR}/FSLib)
add_subdirectory(${PROJECT_SOURCE_DIR}/Archiver)
add_subdirectory(${PROJECT_SOURCE_DIR}/FSTest)
add_subdirectory(${PROJECT_SOURCE_DIR}/PropertyGenerator)
# add_subdirectory(${PROJECT_SOURCE_DIR}/Unpacker)
