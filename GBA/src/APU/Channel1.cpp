#include <GBA/include/APU/Channel1.hpp>
#include <algorithm>
#include <cstring>
#include <functional>
#include <utility>
#include <GBA/include/APU/Constants.hpp>
#include <GBA/include/APU/Registers.hpp>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/Types.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>

namespace audio
{
Channel1::Channel1(EventScheduler& scheduler) : scheduler_(scheduler)
{
    registers_.fill(std::byte{0});
    envelopeIncrease_ = false;
    envelopePace_ = 0;
    currentVolume_ = 0;
    dutyCycleIndex_ = 0;
    lengthTimerExpired_ = false;
    frequencyOverflow_ = false;

    scheduler_.RegisterEvent(EventType::Channel1Clock, std::bind(&Channel1::Clock, this, std::placeholders::_1));
    scheduler_.RegisterEvent(EventType::Channel1Envelope, std::bind(&Channel1::Envelope, this, std::placeholders::_1));
    scheduler_.RegisterEvent(EventType::Channel1LengthTimer, std::bind(&Channel1::LengthTimer, this, std::placeholders::_1));
    scheduler_.RegisterEvent(EventType::Channel1FrequencySweep, std::bind(&Channel1::FrequencySweep, this, std::placeholders::_1));
}

std::pair<u32, bool> Channel1::ReadReg(u32 addr, AccessSize length)
{
    u32 val = ReadMemoryBlock(registers_, addr, CHANNEL_1_ADDR_MIN, length);
    return {val, false};
}

bool Channel1::WriteReg(u32 addr, u32 val, AccessSize length)
{
    WriteMemoryBlock(registers_, addr, CHANNEL_1_ADDR_MIN, val, length);
    auto sound1cnt = GetSOUND1CNT();
    bool triggered = sound1cnt.trigger;

    if (triggered)
    {
        sound1cnt.trigger = 0;
        SetSOUND1CNT(sound1cnt);
        Start(sound1cnt);
    }

    std::memset(&registers_[6], 0, 2);
    return triggered;
}

u8 Channel1::Sample() const
{
    if (lengthTimerExpired_ || frequencyOverflow_)
    {
        return 0;
    }

    return currentVolume_ * DUTY_CYCLE[GetSOUND1CNT().waveDuty][dutyCycleIndex_];
}

void Channel1::Start(SOUND1CNT sound1cnt)
{
    // Set latched registers
    envelopeIncrease_ = sound1cnt.envelopeDirection;
    envelopePace_ = sound1cnt.envelopePace;

    // Set initial status
    currentVolume_ = sound1cnt.initialVolume;
    dutyCycleIndex_ = 0;
    lengthTimerExpired_ = false;
    frequencyOverflow_ = false;

    // Unschedule any existing events
    scheduler_.UnscheduleEvent(EventType::Channel1Clock);
    scheduler_.UnscheduleEvent(EventType::Channel1Envelope);
    scheduler_.UnscheduleEvent(EventType::Channel1LengthTimer);
    scheduler_.UnscheduleEvent(EventType::Channel1FrequencySweep);

    // Schedule relevant events
    scheduler_.ScheduleEvent(EventType::Channel1Clock, ((0x800 - sound1cnt.period) * CPU_CYCLES_PER_GB_CYCLE));

    if (sound1cnt.envelopePace != 0)
    {
        scheduler_.ScheduleEvent(EventType::Channel1Envelope, envelopePace_ * CPU_CYCLES_PER_ENVELOPE_SWEEP);
    }

    if (sound1cnt.lengthEnable)
    {
        int cyclesUntilEvent = (64 - sound1cnt.initialLengthTimer) * CPU_CYCLES_PER_SOUND_LENGTH;
        scheduler_.ScheduleEvent(EventType::Channel1LengthTimer, cyclesUntilEvent);
    }

    u8 sweepPace = std::max(sound1cnt.sweepPace, static_cast<u64>(1));
    scheduler_.ScheduleEvent(EventType::Channel1FrequencySweep, sweepPace * CPU_CYCLES_PER_FREQUENCY_SWEEP);
}

void Channel1::Clock(int extraCycles)
{
    if (lengthTimerExpired_ || frequencyOverflow_)
    {
        return;
    }

    dutyCycleIndex_ = (dutyCycleIndex_ + 1) % 8;
    int cyclesUntilNextEvent =  ((0x800 - GetSOUND1CNT().period) * CPU_CYCLES_PER_GB_CYCLE) - extraCycles;
    scheduler_.ScheduleEvent(EventType::Channel1Clock, cyclesUntilNextEvent);
}

void Channel1::Envelope(int extraCycles)
{
    if (lengthTimerExpired_ || frequencyOverflow_)
    {
        return;
    }

    bool reschedule = true;

    if (envelopeIncrease_ && (currentVolume_ < 0x0F))
    {
        ++currentVolume_;
    }
    else if (!envelopeIncrease_ && (currentVolume_ > 0))
    {
        --currentVolume_;
    }
    else
    {
        reschedule = false;
    }

    if (reschedule)
    {
        int cyclesUntilNextEvent = (envelopePace_ * CPU_CYCLES_PER_ENVELOPE_SWEEP) - extraCycles;
        scheduler_.ScheduleEvent(EventType::Channel1Envelope, cyclesUntilNextEvent);
    }
}

void Channel1::FrequencySweep(int extraCycles)
{
    if (lengthTimerExpired_ || frequencyOverflow_)
    {
        return;
    }

    auto sound1cnt = GetSOUND1CNT();
    u16 currentPeriod = sound1cnt.period;
    u16 delta = currentPeriod / (0x01 << sound1cnt.step);
    u16 updatedPeriod = 0;

    if (sound1cnt.sweepDirection)
    {
        // Subtraction (period decrease)
        if (currentPeriod > delta)
        {
            updatedPeriod = currentPeriod - delta;
        }
    }
    else
    {
        // Addition (period increase)
        updatedPeriod += delta;

        if (updatedPeriod > 0x07FF)
        {
            frequencyOverflow_ = true;
            updatedPeriod = currentPeriod;
        }
    }

    u8 sweepPace = sound1cnt.sweepPace;

    if (sweepPace != 0)
    {
        sound1cnt.period = updatedPeriod;
    }

    sweepPace = std::max(sweepPace, static_cast<u8>(1));

    if (!frequencyOverflow_)
    {
        int cyclesUntilNextEvent = (sweepPace * CPU_CYCLES_PER_FREQUENCY_SWEEP) - extraCycles;
        scheduler_.ScheduleEvent(EventType::Channel1FrequencySweep, cyclesUntilNextEvent);
    }
}
}  // namespace audio
