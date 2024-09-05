#pragma once

#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/Types/Types.hpp>

namespace audio
{
constexpr u32 SAMPLING_FREQUENCY_HZ = 32'768;
constexpr u32 CPU_CYCLES_PER_SAMPLE = cpu::CPU_FREQUENCY_HZ / SAMPLING_FREQUENCY_HZ;

// Maintain audio buffer of 22ms
constexpr size_t BUFFER_SIZE = ((SAMPLING_FREQUENCY_HZ * 22) / 1000) * 2;

constexpr u32 CPU_CYCLES_PER_GB_CYCLE = cpu::CPU_FREQUENCY_HZ / 1'048'576;
constexpr u32 CPU_CYCLES_PER_ENVELOPE_SWEEP = cpu::CPU_FREQUENCY_HZ / 64;
constexpr u32 CPU_CYCLES_PER_SOUND_LENGTH = cpu::CPU_FREQUENCY_HZ / 256;
constexpr u32 CPU_CYCLES_PER_FREQUENCY_SWEEP = cpu::CPU_FREQUENCY_HZ / 128;

constexpr i16 MIN_OUTPUT_LEVEL = 0;
constexpr i16 MAX_OUTPUT_LEVEL = 1023;

constexpr i8 DUTY_CYCLE[4][8] =
{
    {1, 1, 1, 1, 1, 1, 1, 0},
    {1, 1, 1, 1, 1, 1, 0, 0},
    {1, 1, 1, 1, 0, 0, 0, 0},
    {1, 1, 0, 0, 0, 0, 0, 0}
};
}  // namespace audio
