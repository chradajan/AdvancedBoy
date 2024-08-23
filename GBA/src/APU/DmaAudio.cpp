#include <GBA/include/APU/DmaAudio.hpp>
#include <utility>
#include <GBA/include/APU/Registers.hpp>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/Utilities/CicrularBuffer.hpp>
#include <GBA/include/Types.hpp>

namespace audio
{
DmaAudio::DmaAudio()
{
    fifoA_.Clear();
    fifoB_.Clear();
    sampleA_ = 0;
    sampleB_ = 0;
}

void DmaAudio::WriteReg(u32 addr, u32 val, AccessSize length)
{
    u8 sampleCount = static_cast<u8>(length);

    if ((FIFO_A_ADDR <= addr) && (addr < FIFO_B_ADDR))
    {
        FifoPush(fifoA_, val, sampleCount);
    }
    else if ((FIFO_B_ADDR <= addr) && (addr < (FIFO_B_ADDR + 4)))
    {
        FifoPush(fifoB_, val, sampleCount);
    }
}

std::pair<bool, bool> DmaAudio::TimerOverflow(u8 index, SOUNDCNT_H soundcnt_h)
{
    bool replenishA = false;
    bool replenishB = false;

    if (soundcnt_h.dmaTimerSelectA == index)
    {
        if (!fifoA_.Empty())
        {
            sampleA_ = fifoA_.Pop();
        }

        replenishA = fifoA_.Size() < 17;
    }

    if (soundcnt_h.dmaTimerSelectB == index)
    {
        if (!fifoB_.Empty())
        {
            sampleB_ = fifoB_.Pop();
        }

        replenishB = fifoB_.Size() < 17;
    }

    return {replenishA, replenishB};
}

std::pair<i16, i16> DmaAudio::Sample(SOUNDCNT_H soundcnt_h) const
{
    i16 sampleA = sampleA_ * (soundcnt_h.dmaVolumeA ? 4 : 2);
    i16 sampleB = sampleB_ * (soundcnt_h.dmaVolumeB ? 4 : 2);
    return {sampleA, sampleB};
}

void DmaAudio::CheckFifoClear(SOUNDCNT_H& soundcnt_h)
{
    if (soundcnt_h.dmaResetA)
    {
        fifoA_.Clear();
        sampleA_ = 0;
        soundcnt_h.dmaResetA = 0;
    }

    if (soundcnt_h.dmaResetB)
    {
        fifoB_.Clear();
        sampleB_ = 0;
        soundcnt_h.dmaResetB = 0;
    }
}

void DmaAudio::FifoPush(DmaSoundFifo& fifo, u32 val, u8 sampleCount)
{
    while ((sampleCount > 0) && !fifo.Full())
    {
        i8 sample = val & U8_MAX;
        fifo.Push(sample);
        val >>= 8;
        --sampleCount;
    }
}
}  // namespace audio
