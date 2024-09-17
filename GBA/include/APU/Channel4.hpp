#pragma once

#include <array>
#include <bit>
#include <utility>
#include <GBA/include/APU/Registers.hpp>
#include <GBA/include/Utilities/Types.hpp>

class EventScheduler;

namespace audio
{
class Channel4
{
public:
    Channel4() = delete;
    Channel4(Channel4 const&) = delete;
    Channel4& operator=(Channel4 const&) = delete;
    Channel4(Channel4&&) = delete;
    Channel4& operator=(Channel4&&) = delete;

    /// @brief Initialize channel 4.
    /// @param scheduler Reference to scheduler to post audio events to.
    Channel4(EventScheduler& scheduler);

    /// @brief Read a Channel 4 register.
    /// @param addr Address of register(s) to read.
    /// @param length Memory access size of the read.
    /// @return Value of register and whether this read triggered open bus behavior.
    std::pair<u32, bool> ReadReg(u32 addr, AccessSize length);

    /// @brief Write a Channel 4 register.
    /// @param addr Address of register(s) to write.
    /// @param value Value to write to register(s).
    /// @param length Memory access size of the write.
    /// @return Whether this write triggered this channel to start/restart.
    bool WriteReg(u32 addr, u32 val, AccessSize length);

    /// @brief Sample Channel 4's current output.
    /// @return Channel 4 output value.
    u8 Sample() const;

    /// @brief Check if Channel 4 has turned off due to its length timer expiring.
    /// @return True if length timer has expired.
    bool Expired() const { return lengthTimerExpired_; }

private:
    /// @brief Start Channel 4 processing.
    /// @param sound4cnt SOUND4CNT register value.
    void Start(SOUND4CNT sound4cnt);

    /// @brief Calculate how many CPU cycles until Channel 4 should be clocked again based on its frequency.
    /// @param sound4cnt SOUND4CNT register value.
    /// @return Number of cycles between clocks.
    int EventCycles(SOUND4CNT sound4cnt) const;

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Event callbacks
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Callback function to advance Channel 4's shift register.
    /// @param extraCycles Number of cycles since this callback was supposed to execute.
    void Clock(int extraCycles);

    /// @brief Callback function to advance Channel 4's envelope.
    /// @param extraCycles Number of cycles since this callback was supposed to execute.
    void Envelope(int extraCycles);

    /// @brief Callback function to disable Channel 4 if its length timer was enabled when it was triggered.
    void LengthTimer(int) { lengthTimerExpired_ = true; };

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Register access/updates
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// SOUND4CNT L/H/X getter/setter
    SOUND4CNT GetSOUND4CNT() const { return std::bit_cast<SOUND4CNT, std::array<std::byte, 8>>(registers_); }
    void SetSOUND4CNT(SOUND4CNT reg) { registers_ = std::bit_cast<std::array<std::byte, 8>, SOUND4CNT>(reg); }

    // Registers
    std::array<std::byte, 8> registers_;

    // Latched values
    bool envelopeIncrease_;
    u8 envelopePace_;

    // State
    u8 currentVolume_;
    bool lengthTimerExpired_;

    // Shift register
    u16 lsfr_;

    // External components
    EventScheduler& scheduler_;
};
}  // namespace audio
