#pragma once

#include <array>
#include <bit>
#include <utility>
#include <GBA/include/APU/Registers.hpp>
#include <GBA/include/Utilities/Types.hpp>

class EventScheduler;
namespace debug { class APUDebugger; }

namespace audio
{
class Channel1
{
public:
    Channel1() = delete;
    Channel1(Channel1 const&) = delete;
    Channel1& operator=(Channel1 const&) = delete;
    Channel1(Channel1&&) = delete;
    Channel1& operator=(Channel1&&) = delete;

    /// @brief Initialize channel 1.
    /// @param scheduler Reference to scheduler to post audio events to.
    Channel1(EventScheduler& scheduler);

    /// @brief Read a Channel 1 register.
    /// @param addr Address of register(s) to read.
    /// @param length Memory access size of the read.
    /// @return Value of register and whether this read triggered open bus behavior.
    std::pair<u32, bool> ReadReg(u32 addr, AccessSize length);

    /// @brief Write a Channel 1 register.
    /// @param addr Address of register(s) to write.
    /// @param value Value to write to register(s).
    /// @param length Memory access size of the write.
    /// @return Whether this write triggered this channel to start/restart.
    bool WriteReg(u32 addr, u32 val, AccessSize length);

    /// @brief Sample Channel 1's current output.
    /// @return Channel 1 output value.
    u8 Sample() const;

    /// @brief Check if Channel 1 has turned off due to its length timer expiring.
    /// @return True if length timer has expired.
    bool Expired() const { return lengthTimerExpired_; }

private:
    /// @brief Start Channel 1 processing.
    /// @param sound1cnt SOUND1CNT register value.
    void Start(SOUND1CNT sound1cnt);

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Event callbacks
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Callback function to advance Channel 1's duty cycle step.
    /// @param extraCycles Number of cycles since this callback was supposed to execute.
    void Clock(int extraCycles);

    /// @brief Callback function to advance Channel 1's envelope.
    /// @param extraCycles Number of cycles since this callback was supposed to execute.
    void Envelope(int extraCycles);

    /// @brief Callback function to disable Channel 1 if its length timer was enabled when it was triggered.
    void LengthTimer(int) { lengthTimerExpired_ = true; };

    /// @brief Callback function to advance Channel 1's frequency sweep.
    /// @param extraCycles Number of cycles since this callback was supposed to execute.
    void FrequencySweep(int extraCycles);

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Register access/updates
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// SOUND1CNT L/H/X getter/setter
    SOUND1CNT GetSOUND1CNT() const { return std::bit_cast<SOUND1CNT, std::array<std::byte, 8>>(registers_); }
    void SetSOUND1CNT(SOUND1CNT reg) { registers_ = std::bit_cast<std::array<std::byte, 8>, SOUND1CNT>(reg); }

    // Registers
    std::array<std::byte, 8> registers_;

    // Latched values
    bool envelopeIncrease_;
    u8 envelopePace_;

    // State
    u8 currentVolume_;
    u8 dutyCycleIndex_;
    bool lengthTimerExpired_;
    bool frequencyOverflow_;

    // External components
    EventScheduler& scheduler_;

    // Debug
    friend class debug::APUDebugger;
};
}  // namespace audio
