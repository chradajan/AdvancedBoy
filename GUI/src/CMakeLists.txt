project(AdvancedBoy)

target_sources(${PROJECT_NAME} PRIVATE
    Bindings.cpp
    EmuThread.cpp
    FlowLayout.cpp
    GamepadListener.cpp
    GBA.cpp
    LCD.cpp
    main.cpp
    MainWindow.cpp
    PersistentData.cpp
)

add_subdirectory(DebugWindows)
add_subdirectory(Settings)
