cmake_minimum_required(VERSION 3.2)
project(cryptlib CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ./lib)

include_directories("../../../extern/include")	
include_directories("./include/")

set(CRYPTLIB_HEADERS
	${PROJECT_SOURCE_DIR}/include/aes.h
	${PROJECT_SOURCE_DIR}/include/aes256_decrypt.h
	${PROJECT_SOURCE_DIR}/include/aes256_encrypt.h
	${PROJECT_SOURCE_DIR}/include/big_int.h
	${PROJECT_SOURCE_DIR}/include/cl_endian.h
	${PROJECT_SOURCE_DIR}/include/DataBuffer.h
	${PROJECT_SOURCE_DIR}/include/Exception.h
	${PROJECT_SOURCE_DIR}/include/File.h
	${PROJECT_SOURCE_DIR}/include/iodevice.h
	${PROJECT_SOURCE_DIR}/include/iodevice_provider.h
	${PROJECT_SOURCE_DIR}/include/iodevice_provider_file.h
	${PROJECT_SOURCE_DIR}/include/md5.h
	${PROJECT_SOURCE_DIR}/include/Random.h
	${PROJECT_SOURCE_DIR}/include/RSA.h
	${PROJECT_SOURCE_DIR}/include/Secret.h
	${PROJECT_SOURCE_DIR}/include/sha.h
	${PROJECT_SOURCE_DIR}/include/sha256.h
)
set(CRYPTLIB_SOURCES
	${PROJECT_SOURCE_DIR}/src/aes.cpp
	${PROJECT_SOURCE_DIR}/src/aes256_decrypt.cpp
	${PROJECT_SOURCE_DIR}/src/aes256_encrypt.cpp
	${PROJECT_SOURCE_DIR}/src/big_int.cpp
	${PROJECT_SOURCE_DIR}/src/DataBuffer.cpp
	${PROJECT_SOURCE_DIR}/src/endianess.cpp
	${PROJECT_SOURCE_DIR}/src/Exception.cpp
	${PROJECT_SOURCE_DIR}/src/File.cpp
	${PROJECT_SOURCE_DIR}/src/iodevice.cpp
	${PROJECT_SOURCE_DIR}/src/iodevice_provider_file.cpp
	${PROJECT_SOURCE_DIR}/src/md5.cpp
	${PROJECT_SOURCE_DIR}/src/Random.cpp
	${PROJECT_SOURCE_DIR}/src/RSA.cpp
	${PROJECT_SOURCE_DIR}/src/Secret.cpp
	${PROJECT_SOURCE_DIR}/src/sha.cpp
	${PROJECT_SOURCE_DIR}/src/sha256.cpp
)

add_definitions(-DNOMINMAX)

add_library(${PROJECT_NAME} STATIC ${CRYPTLIB_HEADERS} ${CRYPTLIB_SOURCES})
