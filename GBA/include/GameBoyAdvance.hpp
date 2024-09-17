#pragma once

#include <array>
#include <cstddef>
#include <filesystem>
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
    /// @param vBlankCallback Function to be called whenever the GBA enters VBlank.
    /// @param breakpointCallback Function to be called whenever the GBA encounters a breakpoint set in the CPU debugger.
    explicit GameBoyAdvance(fs::path biosPath,
                            fs::path romPath,
                            std::function<void()> vBlankCallback,
                            std::function<void()> breakpointCallback);

    /// @brief Save backup media to disk.
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

    /// @brief Run the emulator for a single CPU instruction.
    void StepCPU();

    /// @brief Run the emulator until the next time it hits VBlank.
    void StepFrame();

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
