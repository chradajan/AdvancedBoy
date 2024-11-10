project(AdvancedBoy)

target_sources(${PROJECT_NAME} PRIVATE
    EmuThread.hpp
    GamepadListener.hpp
    LCD.hpp
    MainWindow.hpp
)

add_subdirectory(DebugWindows)
add_subdirectory(Settings)
