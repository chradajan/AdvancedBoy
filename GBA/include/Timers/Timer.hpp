#pragma once

#include <array>
#include <cstddef>
#include <cstring>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Types/Types.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>

namespace timers
{
class Timer
{
public:
    Timer() = delete;
    Timer(Timer const&) = delete;
    Timer& operator=(Timer const&) = delete;
    Timer(Timer&&) = delete;
    Timer& operator=(Timer&&) = delete;

    /// @brief Initialize a single timer.
    /// @param index Which timer this is [0, 3].
    /// @param event Event associated with this timer's overflow event.
    /// @param interrupt Interrupt to be requested when this timer overflows.
    /// @param scheduler Reference to event scheduler to post timer overflow events to.
    /// @param systemControl Reference to system control to post timer interrupts to.
    explicit Timer(u8 index, EventType event, InterruptType interrupt, EventScheduler& scheduler, SystemControl& systemControl);

    /// @brief Read an address mapped to one of this timer's registers.
    /// @param addr Address of register(s) to read.
    /// @param length Memory access size of the read.
    /// @return Number of cycles taken to read, value of the requested register(s), and whether it was an open-bus read.
    MemReadData ReadReg(u32 addr, AccessSize length);

    /// @brief Write to an address mapped to one of this timer's registers.
    /// @param addr Address of register(s) to write.
    /// @param val Value to write to register(s).
    /// @param length Memory access size of the write.
    /// @return NNumber of cycles taken to write.
    int WriteReg(u32 addr, u32 val, AccessSize length);

    /// @brief Process this timer's overflow event and restart the timer.
    /// @param extraCycles Cycles since this timer actually overflowed.
    void HandleOverflow(int extraCycles);

    /// @brief Increment this timer's internal counter when it's in cascade mode.
    void CascadeIncrement();

    /// @brief Check if this timer is in cascade mode.
    /// @return Whether this timer is in cascade mode.
    bool CascadeMode() const { return (timerIndex_ != 0) && GetTIMCNT().countUpTiming; };

private:
    struct TIMCNT
    {
        static constexpr size_t INDEX = 2;

        u16 prescalerSelection  : 2;
        u16 countUpTiming       : 1;
        u16                     : 3;
        u16 irq                 : 1;
        u16 enable              : 1;
        u16                     : 8;
    };

    static_assert(sizeof(TIMCNT) == 2, "TIMCNT must be 2 bytes");

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Register access/updates
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief TIMCNT register access.
    TIMCNT GetTIMCNT() const { return MemCpyInit<TIMCNT>(&registers_[TIMCNT::INDEX]); }

    /// @brief Reload value register access.
    u16 GetReload() const { return MemCpyInit<u16>(&registers_[0]); }

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Timer state
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Reset the internal timer value and schedule an overflow event if not in cascade mode.
    /// @param timcnt TIMCNT register value.
    /// @param firstTime True if this timer was just enabled, false when the timer is restarting after an overflow.
    /// @param extraCycles How many cycles have passed since this timer was supposed to have started.
    void StartTimer(TIMCNT timcnt, bool firstTime, int extraCycles);

    /// @brief Update the internal counter based on how many cycles have elapsed since it started. Does nothing in cascade mode.
    /// @param divider Clock divider based on the prescaler selection.
    void UpdateInternalCounter(u16 divider);

    // Registers
    std::array<std::byte, 4> registers_;
    u16 internalTimer_;

    // Timer info
    u8 const timerIndex_;
    EventType const eventType_;
    InterruptType const interruptType_;

    // External components
    EventScheduler& scheduler_;
    SystemControl& systemControl_;
};
}  // namespace timers
