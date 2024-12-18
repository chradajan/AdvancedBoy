#include <GBA/include/APU/APU.hpp>
#include <algorithm>
#include <array>
#include <cstddef>
#include <fstream>
#include <utility>
#include <GBA/include/APU/Channel1.hpp>
#include <GBA/include/APU/Channel2.hpp>
#include <GBA/include/APU/Channel4.hpp>
#include <GBA/include/APU/Constants.hpp>
#include <GBA/include/APU/DmaAudio.hpp>
#include <GBA/include/APU/Registers.hpp>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/System/ClockManager.hpp>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>
#include <GBA/include/Utilities/RingBuffer.hpp>
#include <GBA/include/Utilities/Types.hpp>

namespace audio
{
APU::APU(ClockManager const& clockMgr, EventScheduler& scheduler) :
    channel1_(clockMgr, scheduler),
    channel2_(clockMgr, scheduler),
    channel3_(clockMgr, scheduler),
    channel4_(clockMgr, scheduler),
    clockMgr_(clockMgr),
    scheduler_(scheduler)
{
    registers_.fill(std::byte{0});

    scheduler_.RegisterEvent(EventType::SampleAPU, [this](int extraCycles){ this->Sample(extraCycles); });
    scheduler.ScheduleEvent(EventType::SampleAPU, clockMgr_.GetCpuCyclesPerSample());
}

MemReadData APU::ReadReg(u32 addr, AccessSize length)
{
    u32 val;
    bool openBus;

    switch (addr)
    {
        case CHANNEL_1_ADDR_MIN ... CHANNEL_1_ADDR_MAX:
            std::tie(val, openBus) = channel1_.ReadReg(addr, length);
            break;
        case CHANNEL_2_ADDR_MIN ... CHANNEL_2_ADDR_MAX:
            std::tie(val, openBus) = channel2_.ReadReg(addr, length);
            break;
        case CHANNEL_3_ADDR_MIN ... CHANNEL_3_ADDR_MAX:
            std::tie(val, openBus) = channel3_.ReadReg(addr, length);
            break;
        case CHANNEL_4_ADDR_MIN ... CHANNEL_4_ADDR_MAX:
            std::tie(val, openBus) = channel4_.ReadReg(addr, length);
            break;
        case APU_CONTROL_ADDR_MIN ... APU_CONTROL_ADDR_MAX:
            std::tie(val, openBus) = ReadCntRegisters(addr, length);
            break;
        case WAVE_RAM_ADDR_MIN ... WAVE_RAM_ADDR_MAX:
            std::tie(val, openBus) = channel3_.ReadWaveRAM(addr, length);
            break;
        case DMA_AUDIO_ADDR_MIN ... DMA_AUDIO_ADDR_MAX:
            std::tie(val, openBus) = dmaFifos_.ReadReg(addr, length);
            break;
        default:
            val = 0;
            openBus = true;
            break;
    }

    return {1, val, openBus};
}

int APU::WriteReg(u32 addr, u32 val, AccessSize length)
{
    switch (addr)
    {
        case CHANNEL_1_ADDR_MIN ... CHANNEL_1_ADDR_MAX:
        {
            if (channel1_.WriteReg(addr, val, length))
            {
                auto soundCnt_X = GetSOUNDCNT_X();
                soundCnt_X.chan1On = 1;
                SetSOUNDCNT_X(soundCnt_X);
            }

            break;
        }
        case CHANNEL_2_ADDR_MIN ... CHANNEL_2_ADDR_MAX:
        {
            if (channel2_.WriteReg(addr, val, length))
            {
                auto soundCnt_X = GetSOUNDCNT_X();
                soundCnt_X.chan2On = 1;
                SetSOUNDCNT_X(soundCnt_X);
            }

            break;
        }
        case CHANNEL_3_ADDR_MIN ... CHANNEL_3_ADDR_MAX:
        {
            if (channel3_.WriteReg(addr, val, length))
            {
                auto soundCnt_X = GetSOUNDCNT_X();
                soundCnt_X.chan3On = 1;
                SetSOUNDCNT_X(soundCnt_X);
            }

            break;
        }
        case CHANNEL_4_ADDR_MIN ... CHANNEL_4_ADDR_MAX:
        {
            if (channel4_.WriteReg(addr, val, length))
            {
                auto soundCnt_X = GetSOUNDCNT_X();
                soundCnt_X.chan4On = 1;
                SetSOUNDCNT_X(soundCnt_X);
            }

            break;
        }
        case APU_CONTROL_ADDR_MIN ... APU_CONTROL_ADDR_MAX:
            WriteCntRegisters(addr, val, length);
            break;
        case WAVE_RAM_ADDR_MIN ... WAVE_RAM_ADDR_MAX:
            channel3_.WriteWaveRAM(addr, val, length);
            break;
        case DMA_AUDIO_ADDR_MIN ... DMA_AUDIO_ADDR_MAX:
            dmaFifos_.WriteReg(addr, val, length);
            break;
        default:
            break;
    }

    return 1;
}

void APU::SetVolume(bool mute, int volume)
{
    if (mute)
    {
        volumeMultiplier_ = 0.0f;
    }
    else
    {
        volumeMultiplier_ = static_cast<float>(volume) / 100.0f;
    }
}

void APU::EnableChannels(bool channel1, bool channel2, bool channel3, bool channel4, bool fifoA, bool fifoB)
{
    channel1Enabled_ = channel1;
    channel2Enabled_ = channel2;
    channel3Enabled_ = channel3;
    channel4Enabled_ = channel4;
    fifoAEnabled_ = fifoA;
    fifoBEnabled_ = fifoB;
}

void APU::Serialize(std::ofstream& saveState) const
{
    SerializeArray(registers_);
    channel1_.Serialize(saveState);
    channel2_.Serialize(saveState);
    channel3_.Serialize(saveState);
    channel4_.Serialize(saveState);
    dmaFifos_.Serialize(saveState);
}

void APU::Deserialize(std::ifstream& saveState)
{
    DeserializeArray(registers_);
    channel1_.Deserialize(saveState);
    channel2_.Deserialize(saveState);
    channel3_.Deserialize(saveState);
    channel4_.Deserialize(saveState);
    dmaFifos_.Deserialize(saveState);
}

std::pair<u32, bool> APU::ReadCntRegisters(u32 addr, AccessSize length)
{
    auto soundCnt_X = GetSOUNDCNT_X();

    if (channel1_.Expired())
    {
        soundCnt_X.chan1On = 0;
    }

    if (channel2_.Expired())
    {
        soundCnt_X.chan2On = 0;
    }

    if (channel3_.Expired())
    {
        soundCnt_X.chan3On = 0;
    }

    if (channel4_.Expired())
    {
        soundCnt_X.chan4On = 0;
    }

    SetSOUNDCNT_X(soundCnt_X);
    u32 val = ReadMemoryBlock(registers_, addr, APU_CONTROL_ADDR_MIN, length);
    return {val, false};
}

void APU::WriteCntRegisters(u32 addr, u32 val, AccessSize length)
{
    auto prevSoundCnt_X = GetSOUNDCNT_X();
    WriteMemoryBlock(registers_, addr, APU_CONTROL_ADDR_MIN, val, length);
    auto currSoundCnt_X = GetSOUNDCNT_X();

    if (prevSoundCnt_X.masterEnable && !currSoundCnt_X.masterEnable)
    {
        ZeroObject(currSoundCnt_X);
        channel1_.MasterDisable();
        channel2_.MasterDisable();
        channel3_.MasterDisable();
        channel4_.MasterDisable();
    }
    else
    {
        // Restore SOUNDCNT_X read only bits
        currSoundCnt_X.chan1On = prevSoundCnt_X.chan1On;
        currSoundCnt_X.chan2On = prevSoundCnt_X.chan2On;
        currSoundCnt_X.chan3On = prevSoundCnt_X.chan3On;
        currSoundCnt_X.chan4On = prevSoundCnt_X.chan4On;
    }

    SetSOUNDCNT_X(currSoundCnt_X);

    // Reset FIFOs if needed
    auto soundCnt_H = GetSOUNDCNT_H();
    dmaFifos_.CheckFifoClear(soundCnt_H);
    SetSOUNDCNT_H(soundCnt_H);

    // Reset unused registers to 0
    std::memset(&registers_[6], 0, 2);
    std::memset(&registers_[10], 0, 2);
}

void APU::Sample(int extraCycles)
{
    scheduler_.ScheduleEvent(EventType::SampleAPU, clockMgr_.GetCpuCyclesPerSample() - extraCycles);

    auto soundCnt_L = GetSOUNDCNT_L();
    auto soundCnt_H = GetSOUNDCNT_H();
    auto soundCnt_X = GetSOUNDCNT_X();
    auto soundBias = GetSOUNDBIAS();

    i16 leftSample = 0;
    i16 rightSample = 0;

    if (soundCnt_X.masterEnable)
    {
        // PSG channel samples
        u16 psgLeftSample = 0;
        u16 psgRightSample = 0;

        // Channel 1
        u8 channel1Sample = channel1Enabled_ ? channel1_.Sample() : 0;

        if (soundCnt_L.chan1EnableLeft)
        {
            psgLeftSample += channel1Sample;
        }

        if (soundCnt_L.chan1EnableRight)
        {
            psgRightSample += channel1Sample;
        }

        // Channel 2
        u8 channel2Sample = channel2Enabled_ ? channel2_.Sample() : 0;

        if (soundCnt_L.chan2EnableLeft)
        {
            psgLeftSample += channel2Sample;
        }

        if (soundCnt_L.chan2EnableRight)
        {
            psgRightSample += channel2Sample;
        }

        // Channel 3
        u8 channel3Sample = channel3Enabled_ ? channel3_.Sample() : 0;

        if (soundCnt_L.chan3EnableLeft)
        {
            psgLeftSample += channel3Sample;
        }

        if (soundCnt_L.chan3EnableRight)
        {
            psgRightSample += channel3Sample;
        }

        // Channel 4
        u8 channel4Sample = channel4Enabled_ ? channel4_.Sample() : 0;

        if (soundCnt_L.chan4EnableLeft)
        {
            psgLeftSample += channel4Sample;
        }

        if (soundCnt_L.chan4EnableRight)
        {
            psgRightSample += channel4Sample;
        }

        // Adjust PSG volume and mix in
        u8 psgMultiplier = 16;

        if (soundCnt_H.psgVolume == 0)
        {
            psgMultiplier = 4;
        }
        else if (soundCnt_H.psgVolume == 1)
        {
            psgMultiplier = 8;
        }

        psgLeftSample *= psgMultiplier;
        psgRightSample *= psgMultiplier;

        leftSample += psgLeftSample;
        rightSample += psgRightSample;

        // DMA samples
        auto [fifoASample, fifoBSample] = dmaFifos_.Sample(soundCnt_H);

        if (!fifoAEnabled_)
        {
            fifoASample = 0;
        }

        if (!fifoBEnabled_)
        {
            fifoBSample = 0;
        }

        if (soundCnt_H.dmaEnableLeftA)
        {
            leftSample += fifoASample;
        }

        if (soundCnt_H.dmaEnableRightA)
        {
            rightSample += fifoASample;
        }

        if (soundCnt_H.dmaEnableLeftB)
        {
            leftSample += fifoBSample;
        }

        if (soundCnt_H.dmaEnableRightB)
        {
            rightSample += fifoBSample;
        }

        // Apply SOUNDBIAS, clamp output, and convert to float
        leftSample += (soundBias.biasLevel << 1);
        rightSample += (soundBias.biasLevel << 1);

        leftSample = std::clamp(leftSample, MIN_OUTPUT_LEVEL, MAX_OUTPUT_LEVEL);
        rightSample = std::clamp(rightSample, MIN_OUTPUT_LEVEL, MAX_OUTPUT_LEVEL);

        float leftOutput = (leftSample - 512) / 512.0;
        float rightOutput = (rightSample - 512) / 512.0;

        float sample[2] = {leftOutput * volumeMultiplier_, rightOutput * volumeMultiplier_};
        sampleBuffer_.Write(sample, 2);
    }
    else
    {
        float sample[2] = {0.0f, 0.0f};
        sampleBuffer_.Write(sample, 2);
    }

    ++sampleCounter_;
}
}  // namespace audio
