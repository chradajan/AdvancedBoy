cmake_minimum_required(VERSION 3.28)

project (AdvancedBoy
    LANGUAGES CXX
    VERSION 1.0
    DESCRIPTION "Game Boy Advance Emulator"
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR/bin})

add_executable(${PROJECT_NAME})
add_subdirectory(GBA)
add_subdirectory(GUI)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    COMPILE_FLAGS "-Wall -Wextra -O2"
    PREFIX ""
)

target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}
)
