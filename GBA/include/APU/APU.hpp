#pragma once

#include <array>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <utility>
#include <GBA/include/APU/Channel1.hpp>
#include <GBA/include/APU/Channel2.hpp>
#include <GBA/include/APU/Channel3.hpp>
#include <GBA/include/APU/Channel4.hpp>
#include <GBA/include/APU/Constants.hpp>
#include <GBA/include/APU/DmaAudio.hpp>
#include <GBA/include/APU/Registers.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>
#include <GBA/include/Utilities/RingBuffer.hpp>
#include <GBA/include/Utilities/Types.hpp>

class ClockManager;
class EventScheduler;
namespace debug { class APUDebugger; }

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
    /// @param clockMgr Reference to clock manager.
    /// @param scheduler Reference to event scheduler to post audio events to.
    explicit APU(ClockManager const& clockMgr, EventScheduler& scheduler);

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

    /// @brief Update DMA audio channels on timer overflow.
    /// @param index Index of timer that overflowed.
    /// @return Pair of bools indicating whether FIFO A and B need to be refilled.
    std::pair<bool, bool> TimerOverflow(u8 index) { return dmaFifos_.TimerOverflow(index, GetSOUNDCNT_H()); }

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

    ///---------------------------------------------------------------------------------------------------------------------------------
    /// Output
    ///---------------------------------------------------------------------------------------------------------------------------------

    /// @brief Adjust the volume output level.
    /// @param mute Whether to mute audio output.
    /// @param volume If not muted, volume level of output [0, 100];
    void SetVolume(bool mute, int volume);

    /// @brief Set whether each channel is enabled.
    /// @param channel1 Whether channel 1 is enabled.
    /// @param channel2 Whether channel 2 is enabled.
    /// @param channel3 Whether channel 3 is enabled.
    /// @param channel4 Whether channel 4 is enabled.
    /// @param fifoA Whether FIFO A is enabled.
    /// @param fifoB Whether FIFO B is enabled.
    void EnableChannels(bool channel1, bool channel2, bool channel3, bool channel4, bool fifoA, bool fifoB);

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Save States
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Write data to save state file.
    /// @param saveState Save state stream to write to.
    void Serialize(std::ofstream& saveState) const;

    /// @brief Load data from save state file.
    /// @param saveState Save state stream to read from.
    void Deserialize(std::ifstream& saveState);

private:
    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Register access/updates
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// SOUNDCNT_L getter/setter
    SOUNDCNT_L GetSOUNDCNT_L() const { return MemCpyInit<SOUNDCNT_L>(&registers_[SOUNDCNT_L::INDEX]); }
    void SetSOUNDCNT_L(SOUNDCNT_L reg) { std::memcpy(&registers_[SOUNDCNT_L::INDEX], &reg, sizeof(SOUNDCNT_L)); }

    /// SOUNDCNT_H getter/setter
    SOUNDCNT_H GetSOUNDCNT_H() const { return MemCpyInit<SOUNDCNT_H>(&registers_[SOUNDCNT_H::INDEX]); }
    void SetSOUNDCNT_H(SOUNDCNT_H reg) { std::memcpy(&registers_[SOUNDCNT_H::INDEX], &reg, sizeof(SOUNDCNT_H)); }

    /// SOUNDCNT_X getter/setter
    SOUNDCNT_X GetSOUNDCNT_X() const { return MemCpyInit<SOUNDCNT_X>(&registers_[SOUNDCNT_X::INDEX]); }
    void SetSOUNDCNT_X(SOUNDCNT_X reg) { std::memcpy(&registers_[SOUNDCNT_X::INDEX], &reg, sizeof(SOUNDCNT_X)); }

    /// SOUNDBIAS getter/setter
    SOUNDBIAS GetSOUNDBIAS() const { return MemCpyInit<SOUNDBIAS>(&registers_[SOUNDBIAS::INDEX]); }
    void SetSOUNDBIAS(SOUNDBIAS reg) { std::memcpy(&registers_[SOUNDBIAS::INDEX], &reg, sizeof(SOUNDBIAS)); }

    /// @brief Read the APU control registers.
    /// @param addr Address of register(s) to read.
    /// @param length Memory access size of the write.
    /// @return Value at specified register(s) and whether this read triggered open bus behavior.
    std::pair<u32, bool> ReadCntRegisters(u32 addr, AccessSize length);

    /// @brief Write the APU control registers.
    /// @param addr Address of register(s) to write.
    /// @param val Value to write to register(s).
    /// @param length Memory access size of the write.
    void WriteCntRegisters(u32 addr, u32 val, AccessSize length);

    /// @brief Callback function to collect sample from APU and store to internal sample buffer.
    /// @param extraCycles Cycles since this event was scheduled to execute.
    void Sample(int extraCycles);

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Channels
    ///-----------------------------------------------------------------------------------------------------------------------------

    Channel1 channel1_;
    Channel2 channel2_;
    Channel3 channel3_;
    Channel4 channel4_;
    DmaAudio dmaFifos_;

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Register data
    ///-----------------------------------------------------------------------------------------------------------------------------

    std::array<std::byte, 12> registers_;

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Other data
    ///-----------------------------------------------------------------------------------------------------------------------------

    // Internal sample buffer
    RingBuffer<float, BUFFER_SIZE> sampleBuffer_;
    u32 sampleCounter_;

    // Output level
    float volumeMultiplier_;
    bool channel1Enabled_;
    bool channel2Enabled_;
    bool channel3Enabled_;
    bool channel4Enabled_;
    bool fifoAEnabled_;
    bool fifoBEnabled_;

    // External components
    ClockManager const& clockMgr_;
    EventScheduler& scheduler_;

    // Debug
    friend class debug::APUDebugger;
};
}  // namespace audio
