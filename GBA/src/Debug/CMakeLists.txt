project(AdvancedBoy)

target_sources(${PROJECT_NAME} PRIVATE
    APUDebugger.cpp
    ArmDisassembler.cpp
    CPUDebugger.cpp
    GameBoyAdvanceDebugger.cpp
    PPUDebugger.cpp
    SystemControlDebugger.cpp
    ThumbDisassembler.cpp
)