#pragma once

#include <utility>
#include <GBA/include/APU/Registers.hpp>
#include <GBA/include/Utilities/CicrularBuffer.hpp>
#include <GBA/include/Types.hpp>

namespace audio
{
class DmaAudio
{
using DmaSoundFifo = CircularBuffer<i8, 32>;

public:
    /// @brief Default constructor.
    DmaAudio();

    /// @brief Read a DMA audio register. Registers are write only so always returns open bus.
    /// @return Always returns 0 and open bus.
    std::pair<u32, bool> ReadReg(u32, AccessSize) { return {0, true}; }

    /// @brief Push samples to a DMA audio FIFO.
    /// @param addr Address of FIFO.
    /// @param val Sample value to push into FIFO.
    /// @param length Number of samples to push.
    void WriteReg(u32 addr, u32 val, AccessSize length);

    /// @brief Pop a sample off of FIFOs connected to the timer that overflowed.
    /// @param index Index of timer that overflowed.
    /// @param soundcnt_h SOUNDCNT_H register value.
    /// @return Pair of bools indicating whether each FIFO needs to be refilled.
    std::pair<bool, bool> TimerOverflow(u8 index, SOUNDCNT_H soundcnt_h);

    /// @brief Sample the current output of each FIFO.
    /// @param soundcnt_h SOUNDCNT_H register value.
    /// @return Current output of each FIFO.
    std::pair<i16, i16> Sample(SOUNDCNT_H soundcnt_h) const;

    /// @brief Check if either FIFO needs to be reset after SOUNDCNT_H was potentially written.
    /// @param soundcnt_h New SOUNDCNT_H value. Clears FIFO reset bits if necessary.
    void CheckFifoClear(SOUNDCNT_H& soundcnt_h);

private:
    /// @brief Push samples into a FIFO.
    /// @param fifo FIFO to push samples into.
    /// @param val Sample value.
    /// @param sampleCount Number of samples to push.
    void FifoPush(DmaSoundFifo& fifo, u32 val, u8 sampleCount);

    DmaSoundFifo fifoA_;
    DmaSoundFifo fifoB_;

    i8 sampleA_;
    i8 sampleB_;
};
}  // namespace audio
