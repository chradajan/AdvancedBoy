#pragma once

#include <array>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <unordered_set>
#include <GBA/include/APU/APU.hpp>
#include <GBA/include/BIOS/BIOSManager.hpp>
#include <GBA/include/Cartridge/GamePak.hpp>
#include <GBA/include/CPU/ARM7TDMI.hpp>
#include <GBA/include/DMA/DmaManager.hpp>
#include <GBA/include/Keypad/Keypad.hpp>
#include <GBA/include/Keypad/Registers.hpp>
#include <GBA/include/PPU/PPU.hpp>
#include <GBA/include/System/ClockManager.hpp>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Timers/TimerManager.hpp>
#include <GBA/include/Utilities/Types.hpp>

namespace debug { class GameBoyAdvanceDebugger; }
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
    /// @param saveDir Path to directory to store save files and save states.
    /// @param vBlankCallback Function to be called whenever the GBA enters VBlank.
    /// @param breakpointCallback Function to be called whenever the GBA encounters a breakpoint set in the CPU debugger.
    /// @param skipBiosIntro Whether to skip BIOS intro animation and skip straight to executing from cartridge.
    explicit GameBoyAdvance(fs::path biosPath,
                            fs::path romPath,
                            fs::path saveDir,
                            std::function<void()> vBlankCallback,
                            std::function<void()> breakpointCallback,
                            bool skipBiosIntro);

    /// @brief Save backup media to disk.
    ~GameBoyAdvance();

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Validity checks
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Check if the BIOS used to initialize the GBA was successfully loaded.
    /// @return True if valid BIOS is loaded.
    bool ValidBiosLoaded() const { return biosMgr_.BiosLoaded(); }

    /// @brief Check if the ROM used to initialize the GBA was successfully loaded.
    /// @return True if valid GamePak is loaded.
    bool ValidGamePakLoaded() const { return gamePak_ != nullptr; }

    ///---------------------------------------------------------------------------------------------------------------------------------
    /// Audio
    ///---------------------------------------------------------------------------------------------------------------------------------

    /// @brief Adjust the GBA volume output level.
    /// @param mute Whether to mute audio output.
    /// @param volume If not muted, volume level of output [0, 100];
    void SetVolume(bool mute, int volume) { apu_.SetVolume(mute, volume); }

    /// @brief Set whether each APU channel is enabled.
    /// @param channel1 Whether channel 1 is enabled.
    /// @param channel2 Whether channel 2 is enabled.
    /// @param channel3 Whether channel 3 is enabled.
    /// @param channel4 Whether channel 4 is enabled.
    /// @param fifoA Whether FIFO A is enabled.
    /// @param fifoB Whether FIFO B is enabled.
    void SetAPUChannels(bool channel1, bool channel2, bool channel3, bool channel4, bool fifoA, bool fifoB)
    {
        apu_.EnableChannels(channel1, channel2, channel3, channel4, fifoA, fifoB);
    }

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Save States
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Write data to save state file.
    /// @param saveState Save state stream to write to.
    void Serialize(std::ofstream& saveState) const;

    /// @brief Load data from save state file.
    /// @param saveState Save state stream to read from.
    void Deserialize(std::ifstream& saveState);

    /// @brief Get the path of the file where backup media will be saved to.
    /// @return Backup media save file path.
    fs::path GetSavePath() const { return gamePak_ ? gamePak_->GetSavePath() : ""; }

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Emulation Control
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Run the emulator until the internal audio buffer is full.
    void Run();

    /// @brief Run the emulator for a single CPU instruction.
    void StepCPU();

    /// @brief Run the emulator until the next time it hits VBlank.
    void StepFrame();

    /// @brief Set the CPU clock speed.
    /// @param clockSpeed New CPU clock speed in Hz.
    void SetCpuClockSpeed(u32 clockSpeed) { clockMgr_.SetCpuClockSpeed(clockSpeed); }

    /// @brief Update the KEYINPUT register based on current user input.
    /// @param keyinput KEYINPUT value.
    void UpdateKeypad(KEYINPUT keyinput) { keypad_.UpdateKeypad(keyinput); }

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Breakpoints
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Add a breakpoint at a specified address. CPU execution will stop when this matches the address of the next
    ///        instruction to be executed by the CPU.
    /// @param breakpoint Address to set breakpoint at.
    void SetBreakpoint(u32 breakpoint) { breakpoints_.insert(breakpoint); }

    /// @brief Remove an address from the current list of breakpoints.
    /// @param breakpoint Address to set breakpoint at.
    void RemoveBreakpoint(u32 breakpoint) { breakpoints_.erase(breakpoint); }

    /// @brief Check if the CPU is about to execute an instruction with a breakpoint set.
    /// @return Whether a breakpoint has been hit.
    bool EncounteredBreakpoint() {
        return breakpoints_.contains(cpu_.GetNextAddrToExecute()) && (breakpointCycle_ != scheduler_.GetTotalElapsedCycles());
    }

    /// @brief Get the list of breakpoints currently set.
    /// @return An unordered set of all current breakpoints.
    std::unordered_set<u32> const& GetBreakpoints() const { return breakpoints_; }

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// GUI getters
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Get a pointer to the pixel data of the most recently completed frame.
    /// @return Pointer to raw pixel data.
    uchar* GetRawFrameBuffer() { return ppu_.GetRawFrameBuffer(); }

    /// @brief Get FPS counter from PPU.
    /// @return Number of times the PPU has entered VBlank since last check.
    int GetFPSCounter() { return ppu_.GetAndResetFPSCounter(); }

    /// @brief Get the title of the ROM currently running.
    /// @return Current ROM title.
    std::string GetTitle() const { return gamePak_ ? gamePak_->GetTitle() : ""; }

    /// @brief Transfer audio samples from internal buffer to external one.
    /// @param buffer Buffer to transfer samples to.
    /// @param cnt Number of samples to transfer.
    void DrainAudioBuffer(float* buffer, size_t cnt) { apu_.DrainBuffer(buffer, cnt); }

    /// @brief Check how many samples are ready to be transferred from the internal buffer.
    /// @return Current number of buffered audio samples.
    size_t AvailableSamples() const { return apu_.AvailableSamples(); }

private:
    /// @brief Main emulation loop.
    /// @param samples Number of audio samples to be generated before returning from main loop.
    /// @return Whether the loop exited early due to encountering a breakpoint.
    bool MainLoop(size_t samples);

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

    /// @brief Timer overflow event handlers.
    /// @param extraCycles Cycles since this event was scheduled to execute.
    void Timer0Overflow(int extraCycles) { TimerOverflow(0, extraCycles); }
    void Timer1Overflow(int extraCycles) { TimerOverflow(1, extraCycles); }
    void Timer2Overflow(int extraCycles) { TimerOverflow(2, extraCycles); }
    void Timer3Overflow(int extraCycles) { TimerOverflow(3, extraCycles); }

    /// @brief Start FIFO DMA transfers if necessary and notify timer manager of the overflow event.
    /// @param index Index of timer that overflowed.
    /// @param extraCycles Cycles since this event was scheduled to execute.
    void TimerOverflow(u8 index, int extraCycles);

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Member data
    ///-----------------------------------------------------------------------------------------------------------------------------

    // Non-components
    ClockManager clockMgr_;
    EventScheduler scheduler_;

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

    // Open bus
    u32 lastSuccessfulFetch_;

    // Breakpoints
    std::unordered_set<u32> breakpoints_;
    u64 breakpointCycle_;
    bool breakOnVBlank_;
    bool hitVBlank_;

    // Callbacks to GUI
    std::function<void()> VBlankCallback;
    std::function<void()> BreakpointCallback;

    // Debugger
    friend class debug::GameBoyAdvanceDebugger;
};
