project(AdvancedBoy)

target_sources(${PROJECT_NAME} PRIVATE
    ArmDisassembler.cpp
    CPUDebugger.cpp
    GameBoyAdvanceDebugger.cpp
    PPUDebugger.cpp
    ThumbDisassembler.cpp
)