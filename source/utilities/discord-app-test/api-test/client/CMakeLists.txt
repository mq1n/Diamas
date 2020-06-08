cmake_minimum_required(VERSION 3.8)
project(DiscordAPITest_Client)
set(EXE_NAME DiscordAPITest_Client)

file(GLOB ClientApp_HEADERS
    "${PROJECT_SOURCE_DIR}/src/*.h"
    "${PROJECT_SOURCE_DIR}/src/*.hpp"
    "${PROJECT_SOURCE_DIR}/../../../../../extern/3rd/fmt/include/*.h"
)
file(GLOB ClientApp_SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.c"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/../../../../../extern/3rd/fmt/src/*.cc"
)
add_executable(${EXE_NAME} ${ClientApp_HEADERS} ${ClientApp_SOURCES})
target_link_libraries(${EXE_NAME} discord_sdk)

ADD_CUSTOM_COMMAND(TARGET ${EXE_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/../../../../../extern/lib_win_x86/discord_game_sdk.dll ${PROJECT_SOURCE_DIR}/../../../../../Bin/discord_game_sdk.dll
)