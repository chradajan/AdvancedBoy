#include <GBA/include/APU/Channel4.hpp>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <functional>
#include <utility>
#include <GBA/include/APU/Constants.hpp>
#include <GBA/include/APU/Registers.hpp>
#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/System/ClockManager.hpp>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>
#include <GBA/include/Utilities/Types.hpp>

namespace audio
{
Channel4::Channel4(ClockManager const& clockMgr, EventScheduler& scheduler) : clockMgr_(clockMgr), scheduler_(scheduler)
{
    registers_.fill(std::byte{0});
    envelopeIncrease_ = false;
    envelopePace_ = 0;
    currentVolume_ = 0;
    lengthTimerExpired_ = false;

    scheduler_.RegisterEvent(EventType::Channel4Clock, std::bind(&Channel4::Clock, this, std::placeholders::_1));
    scheduler_.RegisterEvent(EventType::Channel4Envelope, std::bind(&Channel4::Envelope, this, std::placeholders::_1));
    scheduler_.RegisterEvent(EventType::Channel4LengthTimer, std::bind(&Channel4::LengthTimer, this, std::placeholders::_1));
}

std::pair<u32, bool> Channel4::ReadReg(u32 addr, AccessSize length)
{
    u32 val = ReadMemoryBlock(registers_, addr, CHANNEL_4_ADDR_MIN, length);
    return {val, false};
}

bool Channel4::WriteReg(u32 addr, u32 val, AccessSize length)
{
    WriteMemoryBlock(registers_, addr, CHANNEL_4_ADDR_MIN, val, length);
    auto sound4cnt = GetSOUND4CNT();
    bool triggered = sound4cnt.trigger;

    if (triggered)
    {
        sound4cnt.trigger = 0;
        SetSOUND4CNT(sound4cnt);
        Start(sound4cnt);
    }

    std::memset(&registers_[2], 0, 2);
    std::memset(&registers_[6], 0, 2);
    return triggered;
}

u8 Channel4::Sample() const
{
    if (lengthTimerExpired_)
    {
        return 0;
    }

    return (lsfr_ & 0x0001) * currentVolume_;
}

void Channel4::Serialize(std::ofstream& saveState) const
{
    SerializeArray(registers_);
    SerializeTrivialType(envelopeIncrease_);
    SerializeTrivialType(envelopePace_);
    SerializeTrivialType(currentVolume_);
    SerializeTrivialType(lengthTimerExpired_);
    SerializeTrivialType(lsfr_);
}

void Channel4::Deserialize(std::ifstream& saveState)
{
    DeserializeArray(registers_);
    DeserializeTrivialType(envelopeIncrease_);
    DeserializeTrivialType(envelopePace_);
    DeserializeTrivialType(currentVolume_);
    DeserializeTrivialType(lengthTimerExpired_);
    DeserializeTrivialType(lsfr_);
}

void Channel4::Start(SOUND4CNT sound4cnt)
{
    // Set latched registers
    envelopeIncrease_ = sound4cnt.envelopeDirection;
    envelopePace_ = sound4cnt.envelopePace;

    // Set initial status
    currentVolume_ = sound4cnt.initialVolume;
    lengthTimerExpired_ = false;

    // Unschedule any existing events
    scheduler_.UnscheduleEvent(EventType::Channel4Clock);
    scheduler_.UnscheduleEvent(EventType::Channel4Envelope);
    scheduler_.UnscheduleEvent(EventType::Channel4LengthTimer);

    // Schedule relevant events
    scheduler_.ScheduleEvent(EventType::Channel4Clock, EventCycles(sound4cnt));

    if (envelopePace_ != 0)
    {
        scheduler_.ScheduleEvent(EventType::Channel4Envelope, envelopePace_ * clockMgr_.GetCpuCyclesPerEnvelopeSweep());
    }

    if (sound4cnt.lengthEnable)
    {
        int cyclesUntilEvent = (64 - sound4cnt.initialLengthTimer) * clockMgr_.GetCpuCyclesPerSoundLength();
        scheduler_.ScheduleEvent(EventType::Channel4LengthTimer, cyclesUntilEvent);
    }

    lsfr_ = U16_MAX;
}

int Channel4::EventCycles(SOUND4CNT sound4cnt) const
{
    u8 r = sound4cnt.dividingRatio;
    u8 s = sound4cnt.shiftClockFrequency;
    int frequency = 1;

    if (r == 0)
    {
        frequency = 524288 / (1 << s);
    }
    else
    {
        frequency = 262144 / (r * (1 << s));
    }

    return cpu::CPU_FREQUENCY_HZ / frequency;
}

void Channel4::Clock(int extraCycles)
{
    if (lengthTimerExpired_)
    {
        return;
    }

    auto sound4cnt = GetSOUND4CNT();
    u16 result = (lsfr_ & 0x0001) ^ ((lsfr_ & 0x0002) >> 1);
    lsfr_ = (lsfr_ & 0x7FFF) | (result << 15);

    if (sound4cnt.countWidth)
    {
        lsfr_ = (lsfr_ & 0xFF7F) | (result << 7);
    }

    lsfr_ >>= 1;
    scheduler_.ScheduleEvent(EventType::Channel4Clock, EventCycles(sound4cnt) - extraCycles);
}

void Channel4::Envelope(int extraCycles)
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
        int cyclesUntilNextEvent = (envelopePace_ * clockMgr_.GetCpuCyclesPerEnvelopeSweep()) - extraCycles;
        scheduler_.ScheduleEvent(EventType::Channel4Envelope, cyclesUntilNextEvent);
    }
}
}  // namespace audio
