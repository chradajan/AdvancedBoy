project(AdvancedBoy)

target_sources(${PROJECT_NAME} PRIVATE
    ARM.cpp
    ARM7TDMI.cpp
    ArmDisassembler.cpp
    Registers.cpp
    THUMB.cpp
    ThumbDisassembler.cpp
)