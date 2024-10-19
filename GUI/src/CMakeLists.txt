project(AdvancedBoy)

target_sources(${PROJECT_NAME} PRIVATE
    EmuThread.cpp
    FlowLayout.cpp
    GBA.cpp
    LCD.cpp
    main.cpp
    MainWindow.cpp
    PersistentData.cpp
)

add_subdirectory(DebugWindows)
