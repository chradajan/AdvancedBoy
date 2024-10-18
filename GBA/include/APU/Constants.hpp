#pragma once

#include <GBA/include/Utilities/Types.hpp>

namespace audio
{
constexpr u32 SAMPLING_FREQUENCY_HZ = 32'768;

// Maintain audio buffer of 22ms
constexpr size_t BUFFER_SIZE = ((SAMPLING_FREQUENCY_HZ * 22) / 1000) * 2;

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
