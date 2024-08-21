#pragma once

#include <array>
#include <cstddef>
#include <utility>
#include <GBA/include/APU/Constants.hpp>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/Types.hpp>
#include <GBA/include/Utilities/RingBuffer.hpp>

namespace audio
{
/// @brief Audio Processing Unit.
class APU
{
public:
    APU() = delete;
    APU(APU const&) = delete;
    APU& operator=(APU const&) = delete;
    APU(APU&&) = delete;
    APU& operator=(APU&&) = delete;

    /// @brief Initialize the APU.
    /// @param scheduler Reference to event scheduler to post audio events to.
    explicit APU(EventScheduler& scheduler);

    /// @brief Read an address mapped to APU registers.
    /// @param addr Address of APU register(s).
    /// @param length Memory access size of the read.
    /// @return Number of cycles taken to read, value of the requested register(s), and whether it was an open-bus read.
    MemReadData ReadReg(u32 addr, AccessSize length);

    /// @brief Write to an address mapped to APU registers.
    /// @param addr Address of APU register(s).
    /// @param val Value to write to register(s).
    /// @param length Memory access size of the write.
    /// @return Number of cycles taken to write.
    int WriteReg(u32 addr, u32 val, AccessSize length);

    /// @brief TODO: Update DMA audio channels on timer overflow.
    /// @param index Index of timer that overflowed.
    /// @return Pair of bools indicating whether FIFO A and B need to be refilled.
    std::pair<bool, bool> TimerOverflow(u8 index) { (void)index; return {false, false}; }

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Producer thread functions
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Check how many samples can be generated before the internal buffer is full.
    /// @return Number of samples that can be generated.
    size_t FreeBufferSpace() const { return sampleBuffer_.GetFree() / 2; }

    /// @brief Reset the internal sample counter.
    void ClearSampleCounter() { sampleCounter_ = 0; }

    /// @brief Check how many samples have been collected since the last time the counter was cleared.
    /// @return Number of samples collected. One sample means a left and right sample.
    size_t GetSampleCounter() const { return sampleCounter_; }

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Consumer thread functions
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Move samples from the internal buffer into the provided buffer.
    /// @param buffer Buffer to drain samples into.
    /// @param cnt Number of samples to drain. One sample means a single left or right sample.
    void DrainBuffer(float* buffer, size_t cnt) { sampleBuffer_.Read(buffer, cnt); }

    /// @brief Check how many audio samples are currently stored in the internal buffer.
    /// @return Number of available samples. One sample means a single left or right sample.
    size_t AvailableSamples() const { return sampleBuffer_.GetAvailable(); }

private:
    /// @brief Callback function to collect sample from APU and store to internal sample buffer.
    /// @param extraCycles Cycles since this event was scheduled to execute.
    void Sample(int extraCycles);

    std::array<std::byte, 0x48> registers_;

    // Internal sample buffer
    RingBuffer<float, BUFFER_SIZE> sampleBuffer_;
    u32 sampleCounter_;

    // External components
    EventScheduler& scheduler_;
};
}  // namespace audio
