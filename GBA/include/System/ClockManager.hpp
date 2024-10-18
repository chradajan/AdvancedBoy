#pragma once

#include <GBA/include/APU/Constants.hpp>
#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/Utilities/Types.hpp>

class ClockManager
{
public:
    /// @brief Initialize the ClockManager using the default CPU clock speed.
    ClockManager() { SetCpuClockSpeed(cpu::CPU_FREQUENCY_HZ); }

    /// @brief Set the CPU clock speed.
    /// @param clockSpeed New CPU clock speed in Hz.
    void SetCpuClockSpeed(u32 clockSpeed)
    {
        cpuClockSpeed_ = clockSpeed;
        UpdateAllClockSpeeds();
    }

    /// @brief Get the CPU clock speed (Hz).
    u32 GetCpuClockSpeed() const { return cpuClockSpeed_; }

    /// @brief Get the number of CPU cycles to run per APU sample.
    u32 GetCpuCyclesPerSample() const { return cpuCyclesPerSample_; }

    /// @brief Get the ratio of GBA clock speed to GB/GBC clock speed.
    u32 GetCpuCyclesPerGbCycle() const { return cpuCyclesPerGbCycle_; }

    /// @brief Get the number of CPU cycles to run for each envelope sweep period.
    u32 GetCpuCyclesPerEnvelopeSweep() const { return cpuCyclesPerEnvelopeSweep_; }

    /// @brief Get the number of CPU cycles to run for each sound length period.
    u32 GetCpuCyclesPerSoundLength() const { return cpuCyclesPerSoundLength_; }

    /// @brief Get the number of CPU cycles to run for each frequency sweep period.
    u32 GetCpuCyclesPerFrequencySweep() const { return cpuCyclesPerFrequencySweep_; }

private:
    /// @brief Update all speeds/ratios based on the current CPU clock speed.
    void UpdateAllClockSpeeds()
    {
        cpuCyclesPerSample_ = cpuClockSpeed_ / audio::SAMPLING_FREQUENCY_HZ;
        cpuCyclesPerGbCycle_ = cpuClockSpeed_ / 1'048'576;
        cpuCyclesPerEnvelopeSweep_ = cpuClockSpeed_ / 64;
        cpuCyclesPerSoundLength_ = cpuClockSpeed_ / 256;
        cpuCyclesPerFrequencySweep_ = cpuClockSpeed_ / 128;
    }

    u32 cpuClockSpeed_;
    u32 cpuCyclesPerSample_;
    u32 cpuCyclesPerGbCycle_;
    u32 cpuCyclesPerEnvelopeSweep_;
    u32 cpuCyclesPerSoundLength_;
    u32 cpuCyclesPerFrequencySweep_;
};
