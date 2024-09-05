#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

// Unsigned fixed-width integer aliases

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using size_t = std::size_t;

// Signed fixed-width integer aliases

using i8  = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

// Buffer type
using uchar = unsigned char;

// System types

enum class AccessSize : u8
{
    BYTE = 1,
    HALFWORD = 2,
    WORD = 4,
    INVALID = 0xFF
};

struct MemReadData
{
    int Cycles;
    u32 Value;
    bool OpenBus;
};

// Memory constants

constexpr u32 KiB = 1024;
constexpr u32 MiB = KiB * KiB;

// Bitwise constants

constexpr u8 U8_MAX = std::numeric_limits<u8>::max();
constexpr u16 U16_MAX = std::numeric_limits<u16>::max();
constexpr u32 U32_MAX = std::numeric_limits<u32>::max();
constexpr u64 U64_MAX = std::numeric_limits<u64>::max();

constexpr u8 U8_MSB = 0x01 << (std::numeric_limits<u8>::digits - 1);
constexpr u16 U16_MSB = 0x01 << (std::numeric_limits<u16>::digits - 1);
constexpr u32 U32_MSB = 0x01 << (std::numeric_limits<u32>::digits - 1);
constexpr u64 U64_MSB = u64{0x01} << (std::numeric_limits<u64>::digits - 1);
