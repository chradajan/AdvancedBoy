project(AdvancedBoy)

target_sources(${PROJECT_NAME} PRIVATE
    EmuThread.cpp
    GBA.cpp
    LCD.cpp
    main.cpp
    MainWindow.cpp
)

add_subdirectory(DebugWindows)
