project(AdvancedBoy)

target_sources(${PROJECT_NAME} PRIVATE
    GameBoyAdvance.cpp
)

add_subdirectory(APU)
add_subdirectory(BIOS)
add_subdirectory(Cartridge)
add_subdirectory(CPU)
add_subdirectory(DMA)
add_subdirectory(Keypad)
add_subdirectory(Memory)
add_subdirectory(PPU)
add_subdirectory(System)
add_subdirectory(Timers)
add_subdirectory(Utilities)
