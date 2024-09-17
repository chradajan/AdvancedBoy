project(AdvancedBoy)

target_sources(${PROJECT_NAME} PRIVATE
    EmuThread.hpp
    LCD.hpp
    MainWindow.hpp
)

add_subdirectory(DebugWindows)
