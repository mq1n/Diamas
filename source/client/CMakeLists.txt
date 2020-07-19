cmake_minimum_required(VERSION 3.2)
project(client)

if (ENABLE_CYTHON)
	add_definitions(-DDISTRIBUTE_CYTHON=1)
	add_definitions(-D__USE_CYTHON__)
endif()

# Kayma sorunu(?) için: add_definitions(-D_WIN32_WINNT_WINXP)
add_definitions(-DNOMINMAX)
add_definitions(-DBUILDING_GRANNY_STATIC=1)

# rpcndr.h(192,14): error C2872:  'byte': ambiguous symbol
# cstddef(24,12): message :  or       'std::byte' 
add_definitions(-D_HAS_STD_BYTE=0)

# Windows headers require the default packing option. Changing this can lead to memory corruption
add_definitions(-DWINDOWS_IGNORE_PACKING_MISMATCH)

# Disable VX instructions for xorstr, otherwise it can cause crash on virtual machine
add_definitions(-DJM_XORSTR_DISABLE_AVX_INTRINSICS)

set(WARNINGS_DISABLE
	# ignore
	5033 #  (cpython/2.7.16) 'register' is no longer a supported storage class 
)

foreach(d ${WARNINGS_DISABLE})
	set(WARNINGS "${WARNINGS} /wd${d}")
endforeach(d)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNINGS} ${MT_BUILD}")

include_directories(${PROJECT_SOURCE_DIR}/../utilities/FileSystem/FSLib/include)

add_subdirectory(${PROJECT_SOURCE_DIR}/eterBase)
if (ENABLE_CYTHON)
	add_subdirectory(${PROJECT_SOURCE_DIR}/eterCythonLib)
endif()
add_subdirectory(${PROJECT_SOURCE_DIR}/eterEffectLib)
add_subdirectory(${PROJECT_SOURCE_DIR}/eterGameLib)
add_subdirectory(${PROJECT_SOURCE_DIR}/eterGrnLib)
add_subdirectory(${PROJECT_SOURCE_DIR}/eterImageLib)
add_subdirectory(${PROJECT_SOURCE_DIR}/eterLib)
add_subdirectory(${PROJECT_SOURCE_DIR}/eterPythonLib)
add_subdirectory(${PROJECT_SOURCE_DIR}/eterSecurity)
add_subdirectory(${PROJECT_SOURCE_DIR}/eterScriptLib)
add_subdirectory(${PROJECT_SOURCE_DIR}/eterSoundLib)
add_subdirectory(${PROJECT_SOURCE_DIR}/eterSphereLib)
add_subdirectory(${PROJECT_SOURCE_DIR}/eterTerrainLib)
add_subdirectory(${PROJECT_SOURCE_DIR}/eterTreeLib)
add_subdirectory(${PROJECT_SOURCE_DIR}/eterWebBrowser)
add_subdirectory(${PROJECT_SOURCE_DIR}/eterXClient)
