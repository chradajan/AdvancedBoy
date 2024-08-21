#include <GBA/include/Timers/Timer.hpp>
#include <bit>
#include <optional>
#include <stdexcept>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>
#include <GBA/include/Types.hpp>

namespace
{
/// @brief Determine the clock divider based on the prescaler selection.
/// @param prescaler TIMCNT prescaler value.
/// @return Clock divider.
u16 GetDivider(u8 prescaler)
{
    switch (prescaler)
    {
        case 0:
            return 1;
        case 1:
            return 64;
        case 2:
            return 256;
        case 3:
            return 1024;
        default:
            break;
    }

    return 1;
}
}  // namespace

namespace timers
{
Timer::Timer(u8 index, EventType event, InterruptType interrupt, EventScheduler& scheduler, SystemControl& systemControl) :
    timerIndex_(index),
    eventType_(event),
    interruptType_(interrupt),
    scheduler_(scheduler),
    systemControl_(systemControl)
{
    registers_.fill(std::byte{0});
    internalTimer_ = 0;
}

MemReadData Timer::ReadReg(u32 addr, AccessSize length)
{
    auto timcnt = GetTIMCNT();
    u8 index = addr & 0x03;
    u32 val;

    if (index < 2)
    {
        if (timcnt.enable)
        {
            UpdateInternalCounter(GetDivider(timcnt.prescalerSelection));
        }

        switch (length)
        {
            case AccessSize::BYTE:
                val = (index == 0) ? (internalTimer_ & U8_MAX) : (internalTimer_ >> 8);
                break;
            case AccessSize::HALFWORD:
                val = internalTimer_;
                break;
            case AccessSize::WORD:
                val = std::bit_cast<u16, TIMCNT>(GetTIMCNT()) | internalTimer_;
                break;
            default:
                throw std::runtime_error("Bad access size");
        }
    }
    else
    {
        val = ReadMemoryBlock(registers_, index, 0, length);
    }

    return {1, val, false};
}

int Timer::WriteReg(u32 addr, u32 val, AccessSize length)
{
    u8 index = addr & 0x03;
    auto prevTimCnt = GetTIMCNT();
    WriteMemoryBlock(registers_, index, 0, val, length);
    auto currTimCnt = GetTIMCNT();

    if (!prevTimCnt.enable && currTimCnt.enable)
    {
        StartTimer(true, 0);
    }
    else if (prevTimCnt.enable && !currTimCnt.enable)
    {
        UpdateInternalCounter(GetDivider(prevTimCnt.prescalerSelection));
        scheduler_.UnscheduleEvent(eventType_);
    }
    else if (prevTimCnt.enable && currTimCnt.enable)
    {
        if (prevTimCnt.countUpTiming && !currTimCnt.countUpTiming)
        {
            StartTimer(true, 0);
        }
        else if (!prevTimCnt.countUpTiming && currTimCnt.countUpTiming)
        {
            UpdateInternalCounter(GetDivider(prevTimCnt.prescalerSelection));
            scheduler_.UnscheduleEvent(eventType_);
        }
    }

    return 1;
}

void Timer::HandleOverflow(int extraCycles)
{
    StartTimer(false, extraCycles);

    if (GetTIMCNT().irq)
    {
        systemControl_.RequestInterrupt(interruptType_);
    }
}

void Timer::CascadeIncrement()
{
    ++internalTimer_;

    if (internalTimer_ == 0)
    {
        scheduler_.ScheduleEvent(eventType_, 0);
    }
}

void Timer::StartTimer(bool firstTime, int extraCycles)
{
    internalTimer_ = GetReload();

    if (!CascadeMode())
    {
        int schedulingOffset = extraCycles * -1;
        u16 divider = GetDivider(GetTIMCNT().prescalerSelection);
        u32 timerLength = (0x0001'0000 - internalTimer_) * divider;

        if (firstTime)
        {
            schedulingOffset += 2;
        }

        scheduler_.ScheduleEvent(eventType_, schedulingOffset, timerLength);
    }
}

void Timer::UpdateInternalCounter(u16 divider)
{
    if (CascadeMode())
    {
        return;
    }

    auto cyclesSinceTimerStarted = scheduler_.ElapsedCycles(eventType_);

    if (cyclesSinceTimerStarted.has_value())
    {
        internalTimer_ = GetReload() + (cyclesSinceTimerStarted.value() / divider);
    }
}
}  // namespace timers
