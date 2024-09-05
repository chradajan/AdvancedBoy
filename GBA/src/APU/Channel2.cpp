#include <GBA/include/APU/Channel2.hpp>
#include <algorithm>
#include <cstring>
#include <functional>
#include <utility>
#include <GBA/include/APU/Constants.hpp>
#include <GBA/include/APU/Registers.hpp>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/Types/Types.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>

namespace audio
{
Channel2::Channel2(EventScheduler& scheduler) : scheduler_(scheduler)
{
    registers_.fill(std::byte{0});
    envelopeIncrease_ = false;
    envelopePace_ = 0;
    currentVolume_ = 0;
    dutyCycleIndex_ = 0;
    lengthTimerExpired_ = false;

    scheduler_.RegisterEvent(EventType::Channel2Clock, std::bind(&Channel2::Clock, this, std::placeholders::_1));
    scheduler_.RegisterEvent(EventType::Channel2Envelope, std::bind(&Channel2::Envelope, this, std::placeholders::_1));
    scheduler_.RegisterEvent(EventType::Channel2LengthTimer, std::bind(&Channel2::LengthTimer, this, std::placeholders::_1));
}

std::pair<u32, bool> Channel2::ReadReg(u32 addr, AccessSize length)
{
    u32 val = ReadMemoryBlock(registers_, addr, CHANNEL_2_ADDR_MIN, length);
    return {val, false};
}

bool Channel2::WriteReg(u32 addr, u32 val, AccessSize length)
{
    WriteMemoryBlock(registers_, addr, CHANNEL_2_ADDR_MIN, val, length);
    auto sound2cnt = GetSOUND2CNT();
    bool triggered = sound2cnt.trigger;

    if (triggered)
    {
        sound2cnt.trigger = 0;
        SetSOUND2CNT(sound2cnt);
        Start(sound2cnt);
    }

    std::memset(&registers_[2], 0, 2);
    std::memset(&registers_[6], 0, 2);
    return triggered;
}

u8 Channel2::Sample() const
{
    if (lengthTimerExpired_)
    {
        return 0;
    }

    return currentVolume_ * DUTY_CYCLE[GetSOUND2CNT().waveDuty][dutyCycleIndex_];
}

void Channel2::Start(SOUND2CNT sound2cnt)
{
    // Set latched registers
    envelopeIncrease_ = sound2cnt.envelopeDirection;
    envelopePace_ = sound2cnt.envelopePace;

    // Set initial status
    currentVolume_ = sound2cnt.initialVolume;
    dutyCycleIndex_ = 0;
    lengthTimerExpired_ = false;

    // Unschedule any existing events
    scheduler_.UnscheduleEvent(EventType::Channel2Clock);
    scheduler_.UnscheduleEvent(EventType::Channel2Envelope);
    scheduler_.UnscheduleEvent(EventType::Channel2LengthTimer);

    // Schedule relevant events
    scheduler_.ScheduleEvent(EventType::Channel2Clock, ((0x800 - sound2cnt.period) * CPU_CYCLES_PER_GB_CYCLE));

    if (sound2cnt.envelopePace != 0)
    {
        scheduler_.ScheduleEvent(EventType::Channel2Envelope, envelopePace_ * CPU_CYCLES_PER_ENVELOPE_SWEEP);
    }

    if (sound2cnt.lengthEnable)
    {
        int cyclesUntilEvent = (64 - sound2cnt.initialLengthTimer) * CPU_CYCLES_PER_SOUND_LENGTH;
        scheduler_.ScheduleEvent(EventType::Channel2LengthTimer, cyclesUntilEvent);
    }
}

void Channel2::Clock(int extraCycles)
{
    if (lengthTimerExpired_)
    {
        return;
    }

    dutyCycleIndex_ = (dutyCycleIndex_ + 1) % 8;
    int cyclesUntilNextEvent =  ((0x800 - GetSOUND2CNT().period) * CPU_CYCLES_PER_GB_CYCLE) - extraCycles;
    scheduler_.ScheduleEvent(EventType::Channel2Clock, cyclesUntilNextEvent);
}

void Channel2::Envelope(int extraCycles)
{
    if (lengthTimerExpired_)
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
        scheduler_.ScheduleEvent(EventType::Channel2Envelope, cyclesUntilNextEvent);
    }
}
}  // namespace audio
