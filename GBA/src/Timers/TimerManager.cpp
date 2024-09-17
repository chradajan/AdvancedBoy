#include <GBA/include/Timers/TimerManager.hpp>
#include <array>
#include <cstddef>
#include <stdexcept>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Timers/Timer.hpp>
#include <GBA/include/Utilities/Types.hpp>

namespace timers
{
TimerManager::TimerManager(EventScheduler& scheduler, SystemControl& systemControl) :
    timers_{
        Timer(0, EventType::Timer0Overflow, InterruptType::TIMER_0_OVERFLOW, scheduler, systemControl),
        Timer(1, EventType::Timer1Overflow, InterruptType::TIMER_1_OVERFLOW, scheduler, systemControl),
        Timer(2, EventType::Timer2Overflow, InterruptType::TIMER_2_OVERFLOW, scheduler, systemControl),
        Timer(3, EventType::Timer3Overflow, InterruptType::TIMER_3_OVERFLOW, scheduler, systemControl),
    }
{
}

MemReadData TimerManager::ReadReg(u32 addr, AccessSize length)
{
    if (addr <= TIMER_0_ADDR_MAX)
    {
        return timers_[0].ReadReg(addr, length);
    }
    else if (addr <= TIMER_1_ADDR_MAX)
    {
        return timers_[1].ReadReg(addr, length);
    }
    else if (addr <= TIMER_2_ADDR_MAX)
    {
        return timers_[2].ReadReg(addr, length);
    }
    else if (addr <= TIMER_3_ADDR_MAX)
    {
        return timers_[3].ReadReg(addr, length);
    }

    throw std::runtime_error("Read invalid Timer address");
}

int TimerManager::WriteReg(u32 addr, u32 val, AccessSize length)
{
    if (addr <= TIMER_0_ADDR_MAX)
    {
        return timers_[0].WriteReg(addr, val, length);
    }
    else if (addr <= TIMER_1_ADDR_MAX)
    {
        return timers_[1].WriteReg(addr, val, length);
    }
    else if (addr <= TIMER_2_ADDR_MAX)
    {
        return timers_[2].WriteReg(addr, val, length);
    }
    else if (addr <= TIMER_3_ADDR_MAX)
    {
        return timers_[3].WriteReg(addr, val, length);
    }

    throw std::runtime_error("Wrote invalid Timer address");
}

void TimerManager::TimerOverflow(u8 index, int extraCycles)
{
    Timer& overflowedTimer = timers_[index];
    Timer* nextTimer = (index < 3) ? &timers_[index + 1] : nullptr;
    overflowedTimer.HandleOverflow(extraCycles);

    if ((nextTimer != nullptr) && (nextTimer->CascadeMode()))
    {
        nextTimer->CascadeIncrement();
    }
}
}  // namespace timers
