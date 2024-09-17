project(AdvancedBoy)

target_sources(${PROJECT_NAME} PRIVATE
    BackgroundViewer.hpp
    CpuDebugger.hpp
    EmuThread.hpp
    LCD.hpp
    MainWindow.hpp
)

add_subdirectory(DebugWindows)
