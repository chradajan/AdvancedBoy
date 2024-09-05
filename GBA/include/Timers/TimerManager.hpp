#pragma once

#include <array>
#include <cstddef>
#include <GBA/include/Timers/Timer.hpp>
#include <GBA/include/Types/Types.hpp>

class EventScheduler;
class SystemControl;

namespace timers
{
/// @brief Manager class for system timers.
class TimerManager
{
public:
    TimerManager() = delete;
    TimerManager(TimerManager const&) = delete;
    TimerManager& operator=(TimerManager const&) = delete;
    TimerManager(TimerManager&&) = delete;
    TimerManager& operator=(TimerManager&&) = delete;

    /// @brief Initialize the timer manager.
    /// @param scheduler Reference to event scheduler to post timer overflow events to.
    /// @param systemControl Reference to system control to post timer interrupts to.
    explicit TimerManager(EventScheduler& scheduler, SystemControl& systemControl);

    /// @brief Read an address mapped to timer registers.
    /// @param addr Address of timer register(s).
    /// @param length Memory access size of the read.
    /// @return Number of cycles taken to read, value of the requested register(s), and whether it was an open-bus read.
    MemReadData ReadReg(u32 addr, AccessSize length);

    /// @brief Write to an address mapped to timer registers.
    /// @param addr Address of timer register(s).
    /// @param val Value to write to register(s).
    /// @param length Memory access size of the write.
    /// @return Number of cycles taken to write.
    int WriteReg(u32 addr, u32 val, AccessSize length);

    /// @brief Handle a timer overflow event.
    /// @param index Index of timer that overflowed.
    /// @param extraCycles How many cycles have passed since the overflow event.
    void TimerOverflow(u8 index, int extraCycles);

private:
    std::array<Timer, 4> timers_;
};
}  // namespace timers
