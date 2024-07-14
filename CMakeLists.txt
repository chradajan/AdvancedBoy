cmake_minimum_required(VERSION 3.19)

project (AdvancedBoy
    LANGUAGES CXX
    VERSION 1.0
    DESCRIPTION "Game Boy Advance Emulator"
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

find_package(Qt6 REQUIRED COMPONENTS Core Gui OpenGL OpenGLWidgets Widgets)

qt_standard_project_setup()
qt_add_executable(${PROJECT_NAME})
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

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Qt6::Widgets
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
