#pragma once

#include <array>
#include <bit>
#include <fstream>
#include <utility>
#include <GBA/include/APU/Registers.hpp>
#include <GBA/include/Utilities/Types.hpp>

class ClockManager;
class EventScheduler;
namespace debug { class APUDebugger; }

namespace audio
{
class Channel3
{
public:
    Channel3() = delete;
    Channel3(Channel3 const&) = delete;
    Channel3& operator=(Channel3 const&) = delete;
    Channel3(Channel3&&) = delete;
    Channel3& operator=(Channel3&&) = delete;

    /// @brief Initialize channel 3.
    /// @param clockMgr Reference to clock manager.
    /// @param scheduler Reference to scheduler to post audio events to.
    explicit Channel3(ClockManager const& clockMgr, EventScheduler& scheduler);

    /// @brief Read a Channel 3 register.
    /// @param addr Address of register(s) to read.
    /// @param length Memory access size of the read.
    /// @return Value of register and whether this read triggered open bus behavior.
    std::pair<u32, bool> ReadReg(u32 addr, AccessSize length);

    /// @brief Write a Channel 3 register.
    /// @param addr Address of register(s) to write.
    /// @param value Value to write to register(s).
    /// @param length Memory access size of the write.
    /// @return Whether this write triggered this channel to start/restart.
    bool WriteReg(u32 addr, u32 val, AccessSize length);

    /// @brief Read from Wave RAM.
    /// @param addr Address to read.
    /// @param length Memory access size of the read.
    /// @return Current value of Wave RAM and whether this read triggered open bus behavior.
    std::pair<u32, bool> ReadWaveRAM(u32 addr, AccessSize length);

    /// @brief Write to Wave RAM.
    /// @param addr Address to write.
    /// @param val Value to write to Wave RAM.
    /// @param length Memory access size of the write.
    void WriteWaveRAM(u32 addr, u32 val, AccessSize length);

    /// @brief Sample Channel 3's current output.
    /// @return Channel 3 output value.
    u8 Sample() const;

    /// @brief Check if Channel 3 has turned off due to its length timer expiring.
    /// @return True if length timer has expired.
    bool Expired() const { return lengthTimerExpired_; }

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Save States
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Write data to save state file.
    /// @param saveState Save state stream to write to.
    void Serialize(std::ofstream& saveState) const;

    /// @brief Load data from save state file.
    /// @param saveState Save state stream to read from.
    void Deserialize(std::ifstream& saveState);

private:
    /// @brief Start Channel 3 processing.
    /// @param sound1cnt SOUND3CNT register value.
    void Start(SOUND3CNT sound3cnt);

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Event callbacks
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Callback function to advance Channel 3's playback position.
    /// @param extraCycles Number of cycles since this callback was supposed to execute.
    void Clock(int extraCycles);

    /// @brief Callback function to disable Channel 3 if its length timer was enabled when it was triggered.
    void LengthTimer(int) { lengthTimerExpired_ = true; };

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Register access/updates
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// SOUND1CNT L/H/X getter/setter
    SOUND3CNT GetSOUND3CNT() const { return std::bit_cast<SOUND3CNT>(registers_); }
    void SetSOUND3CNT(SOUND3CNT reg) { registers_ = std::bit_cast<std::array<std::byte, 8>>(reg); }

    // Registers
    std::array<std::byte, 8> registers_;

    // Banks
    std::array<std::array<std::byte, 16>, 2> waveRAM_;

    // State
    bool lengthTimerExpired_;
    u8 playbackIndex_;
    u8 playbackMask_;
    u8 playbackBank_;

    // External components
    ClockManager const& clockMgr_;
    EventScheduler& scheduler_;

    // Debug
    friend class debug::APUDebugger;
};
}
