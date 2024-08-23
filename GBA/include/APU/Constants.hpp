#pragma once

#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/Types.hpp>

namespace audio
{
constexpr u32 SAMPLING_FREQUENCY_HZ = 32'768;
constexpr u32 CPU_CYCLES_PER_SAMPLE = cpu::CPU_FREQUENCY_HZ / SAMPLING_FREQUENCY_HZ;

// Maintain audio buffer of 22ms
constexpr size_t BUFFER_SIZE = ((SAMPLING_FREQUENCY_HZ * 22) / 1000) * 2;

constexpr i16 MIN_OUTPUT_LEVEL = 0;
constexpr i16 MAX_OUTPUT_LEVEL = 1023;
}  // namespace audio
