#pragma once

#include <array>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <GBA/include/APU/APU.hpp>
#include <GBA/include/BIOS/BIOSManager.hpp>
#include <GBA/include/Cartridge/GamePak.hpp>
#include <GBA/include/CPU/ARM7TDMI.hpp>
#include <GBA/include/DMA/DmaManager.hpp>
#include <GBA/include/Keypad/Keypad.hpp>
#include <GBA/include/PPU/PPU.hpp>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Timers/TimerManager.hpp>
#include <GBA/include/Types.hpp>

namespace fs = std::filesystem;

/// @brief Represents a single GBA.
class GameBoyAdvance
{
public:
    GameBoyAdvance() = delete;

    /// @brief Initialize the GBA.
    /// @param biosPath Path to BIOS ROM file..
    explicit GameBoyAdvance(fs::path biosPath);

private:
    /// @brief Route a memory read to the correct component.
    /// @param addr Address to read from.
    /// @param length Memory access size of the read.
    /// @return Value returned from the read and number of cycles taken to read.
    std::pair<u32, int> ReadMem(u32 addr, AccessSize length);

    /// @brief Route a memory write to the correct component.
    /// @param addr Address to write to.
    /// @param val Value to write to memory.
    /// @param length Memory access size of the write.
    /// @return Number of cycles taken to write.
    int WriteMem(u32 addr, u32 val, AccessSize length);

    /// @brief Read an address in EWRAM.
    /// @param addr Address to read from.
    /// @param length Memory access size of the read.
    /// @return Number of cycles taken to read, value returned from the read, and whether it was an open-bus read.
    MemReadData ReadEWRAM(u32 addr, AccessSize length);

    /// @brief Write to an address in EWRAM.
    /// @param addr Address to write to.
    /// @param val Value to write.
    /// @param length Memory access size of the write.
    /// @return Number of cycles taken to write.
    int WriteEWRAM(u32 addr, u32 val, AccessSize length);

    /// @brief Read an address in IWRAM.
    /// @param addr Address to read from.
    /// @param length Memory access size of the read.
    /// @return Number of cycles taken to read, value returned from the read, and whether it was an open-bus read.
    MemReadData ReadIWRAM(u32 addr, AccessSize length);

    /// @brief Write to an address in IWRAM.
    /// @param addr Address to write to.
    /// @param val Value to write.
    /// @param length Memory access size of the write.
    /// @return Number of cycles taken to write.
    int WriteIWRAM(u32 addr, u32 val, AccessSize length);

    /// @brief Read an address corresponding to an IO register.
    /// @param addr Address to read from.
    /// @param length Memory access size of the read.
    /// @return Number of cycles taken to read, value returned from the read, and whether it was an open-bus read.
    MemReadData ReadIO(u32 addr, AccessSize length);

    /// @brief Write to an address corresponding to an IO register.
    /// @param addr Address to write to.
    /// @param val Value to write.
    /// @param length Memory access size of the write.
    /// @return Number of cycles taken to write.
    int WriteIO(u32 addr, u32 val, AccessSize length);

    // Non-components
    EventScheduler scheduler_;
    SystemControl systemControl_;

    // Components
    audio::APU apu_;
    BIOSManager biosMgr_;
    cpu::ARM7TDMI cpu_;
    dma::DmaManager dmaMgr_;
    Keypad keypad_;
    graphics::PPU ppu_;
    timers::TimerManager timerMgr_;

    // Optional components
    std::unique_ptr<cartridge::GamePak> gamePak_;

    // System memory
    std::array<std::byte, 256 * KiB> EWRAM_;
    std::array<std::byte, 32 * KiB> IWRAM_;
};
