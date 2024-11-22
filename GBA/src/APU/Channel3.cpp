#include <GBA/include/APU/Channel3.hpp>
#include <array>
#include <bit>
#include <cstring>
#include <fstream>
#include <utility>
#include <GBA/include/APU/Registers.hpp>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/System/ClockManager.hpp>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>
#include <GBA/include/Utilities/Types.hpp>

namespace audio
{
Channel3::Channel3(ClockManager const& clockMgr, EventScheduler& scheduler) : clockMgr_(clockMgr), scheduler_(scheduler)
{
    registers_.fill(std::byte{0});
    lengthTimerExpired_ = false;
    playbackIndex_ = 0;
    playbackMask_ = 0xF0;
    playbackBank_ = 0;

    scheduler_.RegisterEvent(EventType::Channel3Clock, [this](int extraCycles){ this->Clock(extraCycles); });
    scheduler_.RegisterEvent(EventType::Channel3LengthTimer, [this](int extraCycles){ this->LengthTimer(extraCycles); });
}

std::pair<u32, bool> Channel3::ReadReg(u32 addr, AccessSize length)
{
    u32 val = ReadMemoryBlock(registers_, addr, CHANNEL_3_ADDR_MIN, length);
    return {val, false};
}

bool Channel3::WriteReg(u32 addr, u32 val, AccessSize length)
{
    auto prevSound3cnt = GetSOUND3CNT();
    WriteMemoryBlock(registers_, addr, CHANNEL_3_ADDR_MIN, val, length);
    auto sound3cnt = GetSOUND3CNT();
    bool triggered = sound3cnt.trigger;

    if (triggered)
    {
        sound3cnt.trigger = 0;
        SetSOUND3CNT(sound3cnt);
        Start(sound3cnt);
    }
    else if (prevSound3cnt.bankNum != sound3cnt.bankNum)
    {
        playbackBank_ = sound3cnt.bankNum;
    }

    std::memset(&registers_[6], 0, 2);
    return triggered;
}

void Channel3::MasterDisable()
{
    registers_.fill(std::byte{0});
    scheduler_.UnscheduleEvent(EventType::Channel3Clock);
    scheduler_.UnscheduleEvent(EventType::Channel3LengthTimer);
}

std::pair<u32, bool> Channel3::ReadWaveRAM(u32 addr, AccessSize length)
{
    u8 bank = (GetSOUND3CNT().bankNum == 0) ? 1 : 0;
    u32 val = ReadMemoryBlock(waveRAM_[bank], addr, WAVE_RAM_ADDR_MIN, length);
    return {val, false};
}

void Channel3::WriteWaveRAM(u32 addr, u32 val, AccessSize length)
{
    u8 bank = (GetSOUND3CNT().bankNum == 0) ? 1 : 0;
    WriteMemoryBlock(waveRAM_[bank], addr, WAVE_RAM_ADDR_MIN, val, length);
}

u8 Channel3::Sample() const
{
    auto sound3cnt = GetSOUND3CNT();

    if (lengthTimerExpired_ || !sound3cnt.playback || (!sound3cnt.forceVolume && !sound3cnt.soundVolume))
    {
        return 0;
    }

    u8 sample = static_cast<u8>(waveRAM_[playbackBank_][playbackIndex_]) & playbackMask_;

    if (playbackMask_ == 0xF0)
    {
        sample >>= 4;
    }

    if (sound3cnt.forceVolume)
    {
        sample *= 3;
        sample >>= 2;
    }
    else if (sound3cnt.soundVolume == 2)
    {
        sample >>= 1;
    }
    else if (sound3cnt.soundVolume == 3)
    {
        sample >>= 2;
    }

    return sample;
}

void Channel3::Serialize(std::ofstream& saveState) const
{
    SerializeArray(registers_);

    for (auto const& bank : waveRAM_)
    {
        SerializeArray(bank);
    }

    SerializeTrivialType(lengthTimerExpired_);
    SerializeTrivialType(playbackIndex_);
    SerializeTrivialType(playbackMask_);
    SerializeTrivialType(playbackBank_);
}

void Channel3::Deserialize(std::ifstream& saveState)
{
    DeserializeArray(registers_);

    for (auto& bank : waveRAM_)
    {
        DeserializeArray(bank);
    }

    DeserializeTrivialType(lengthTimerExpired_);
    DeserializeTrivialType(playbackIndex_);
    DeserializeTrivialType(playbackMask_);
    DeserializeTrivialType(playbackBank_);
}

void Channel3::Start(SOUND3CNT sound3cnt)
{
    // Set initial state
    lengthTimerExpired_ = false;
    playbackIndex_ = 0;
    playbackMask_ = 0xF0;
    playbackBank_ = sound3cnt.bankNum;

    // Unschedule any existing events
    scheduler_.UnscheduleEvent(EventType::Channel3Clock);
    scheduler_.UnscheduleEvent(EventType::Channel3LengthTimer);

    // Schedule relevant events
    int cyclesUntilClockEvent = (0x0800 - sound3cnt.period) * (clockMgr_.GetCpuCyclesPerGbCycle() / 2);
    scheduler_.ScheduleEvent(EventType::Channel3Clock, cyclesUntilClockEvent);

    if (sound3cnt.lengthEnable)
    {
        int cyclesUntilLengthEvent = (256 - sound3cnt.initialLengthTimer) * clockMgr_.GetCpuCyclesPerSoundLength();
        scheduler_.ScheduleEvent(EventType::Channel3LengthTimer, cyclesUntilLengthEvent);
    }
}

void Channel3::Clock(int extraCycles)
{
    if (lengthTimerExpired_)
    {
        return;
    }

    auto sound3cnt = GetSOUND3CNT();
    playbackMask_ = std::rotr(playbackMask_, 4);

    if (playbackMask_ == 0xF0)
    {
        if (playbackIndex_ == 15)
        {
            playbackIndex_ = 0;

            if (sound3cnt.dimension == 1)
            {
                playbackBank_ ^= 0x01;
            }
        }
        else
        {
            ++playbackIndex_;
        }
    }

    int cyclesUntilNextEvent = ((0x0800 - sound3cnt.period) * (clockMgr_.GetCpuCyclesPerGbCycle() / 2)) - extraCycles;
    scheduler_.ScheduleEvent(EventType::Channel3Clock, cyclesUntilNextEvent);
}
}
