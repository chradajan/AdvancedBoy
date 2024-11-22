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
class Channel2
{
public:
    Channel2() = delete;
    Channel2(Channel2 const&) = delete;
    Channel2& operator=(Channel2 const&) = delete;
    Channel2(Channel2&&) = delete;
    Channel2& operator=(Channel2&&) = delete;

    /// @brief Initialize channel 2.
    /// @param clockMgr Reference to clock manager.
    /// @param scheduler Reference to scheduler to post audio events to.
    explicit Channel2(ClockManager const& clockMgr, EventScheduler& scheduler);

    /// @brief Read a Channel 2 register.
    /// @param addr Address of register(s) to read.
    /// @param length Memory access size of the read.
    /// @return Value of register and whether this read triggered open bus behavior.
    std::pair<u32, bool> ReadReg(u32 addr, AccessSize length);

    /// @brief Write a Channel 2 register.
    /// @param addr Address of register(s) to write.
    /// @param value Value to write to register(s).
    /// @param length Memory access size of the write.
    /// @return Whether this write triggered this channel to start/restart.
    bool WriteReg(u32 addr, u32 val, AccessSize length);

    /// @brief Clear all registers when SOUNDCNT_X master enable is cleared.
    void MasterDisable();

    /// @brief Sample Channel 2's current output.
    /// @return Channel 2 output value.
    u8 Sample() const;

    /// @brief Check if Channel 2 has turned off due to its length timer expiring.
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
    /// @brief Start Channel 2 processing.
    /// @param sound2cnt SOUND2CNT register value.
    void Start(SOUND2CNT sound2cnt);

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Event callbacks
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Callback function to advance Channel 2's duty cycle step.
    /// @param extraCycles Number of cycles since this callback was supposed to execute.
    void Clock(int extraCycles);

    /// @brief Callback function to advance Channel 2's envelope.
    /// @param extraCycles Number of cycles since this callback was supposed to execute.
    void Envelope(int extraCycles);

    /// @brief Callback function to disable Channel 2 if its length timer was enabled when it was triggered.
    void LengthTimer(int) { lengthTimerExpired_ = true; };

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Register access/updates
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// SOUND2CNT L/H/X getter/setter
    SOUND2CNT GetSOUND2CNT() const { return std::bit_cast<SOUND2CNT, std::array<std::byte, 8>>(registers_); }
    void SetSOUND2CNT(SOUND2CNT reg) { registers_ = std::bit_cast<std::array<std::byte, 8>, SOUND2CNT>(reg); }

    // Registers
    std::array<std::byte, 8> registers_;

    // Latched values
    bool envelopeIncrease_;
    u8 envelopePace_;

    // State
    u8 currentVolume_;
    u8 dutyCycleIndex_;
    bool lengthTimerExpired_;

    // External components
    ClockManager const& clockMgr_;
    EventScheduler& scheduler_;

    // Debug
    friend class debug::APUDebugger;
};
}  // namespace audio
