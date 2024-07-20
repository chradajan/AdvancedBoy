#pragma once

#include <array>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <GBA/include/APU/APU.hpp>
#include <GBA/include/BIOS/BIOSManager.hpp>
#include <GBA/include/Cartridge/GamePak.hpp>
#include <GBA/include/CPU/ARM7TDMI.hpp>
#include <GBA/include/DMA/DmaManager.hpp>
#include <GBA/include/Keypad/Keypad.hpp>
#include <GBA/include/Logging/Logger.hpp>
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
    GameBoyAdvance(GameBoyAdvance const&) = delete;
    GameBoyAdvance& operator=(GameBoyAdvance const&) = delete;
    GameBoyAdvance(GameBoyAdvance&&) = delete;
    GameBoyAdvance& operator=(GameBoyAdvance&&) = delete;

    /// @brief Initialize the GBA.
    /// @param biosPath Path to BIOS ROM file.
    /// @param romPath Path to GamePak ROM file.
    /// @param logDir Path to directory where log file should be generated. Pass empty path to disable logging.
    explicit GameBoyAdvance(fs::path biosPath, fs::path romPath, fs::path logDir);

    /// @brief Dump any unlogged entries.
    ~GameBoyAdvance();

    /// @brief Get a pointer to the pixel data of the most recently completed frame.
    /// @return Pointer to raw pixel data.
    uchar* GetRawFrameBuffer() { return ppu_.GetRawFrameBuffer(); }

    /// @brief Get FPS counter from PPU.
    /// @return Number of times the PPU has entered VBlank since last check.
    int GetFPSCounter() { return ppu_.GetAndResetFPSCounter(); }

    /// @brief Get the title of the ROM currently running.
    /// @return Current ROM title.
    std::string GetTitle() const { return gamePak_ ? gamePak_->GetTitle() : ""; }

    /// @brief Update the KEYINPUT register based on current user input.
    /// @param keyinput KEYINPUT value.
    void UpdateKeypad(KEYINPUT keyinput) { keypad_.UpdateKeypad(keyinput); }

    /// @brief Transfer audio samples from internal buffer to external one.
    /// @param buffer Buffer to transfer samples to.
    /// @param cnt Number of samples to transfer.
    void DrainAudioBuffer(float* buffer, size_t cnt) { apu_.DrainBuffer(buffer, cnt); }

    /// @brief Check how many samples are ready to be transferred from the internal buffer.
    /// @return Current number of buffered audio samples.
    size_t AvailableSamples() const { return apu_.AvailableSamples(); }

    /// @brief Run the emulator until the internal audio buffer is full.
    void Run();

private:
    /// @brief Main emulation loop.
    /// @param samples Number of audio samples to be generated before returning from main loop.
    void MainLoop(size_t samples);

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Bus functionality
    ///-----------------------------------------------------------------------------------------------------------------------------

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

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Event handling
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief HBlank event handler.
    /// @param extraCycles Cycles since this event was scheduled to execute.
    void HBlank(int extraCycles);

    /// @brief VBlank event handler.
    /// @param extraCycles Cycles since this event was scheduled to execute.
    void VBlank(int extraCycles);

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Member data
    ///-----------------------------------------------------------------------------------------------------------------------------

    // Non-components
    EventScheduler scheduler_;

    // Debug
    logging::Logger log_;

    // Components
    SystemControl systemControl_;
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
