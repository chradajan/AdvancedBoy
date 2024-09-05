#pragma once

#include <GBA/include/Types/Types.hpp>

namespace cpu
{
constexpr u32 CPU_FREQUENCY_HZ = 16'777'216;

// Special general purpose register indexes

constexpr u8 SP_INDEX = 13;
constexpr u8 LR_INDEX = 14;
constexpr u8 PC_INDEX = 15;

// Vectors

constexpr u32 RESET_VECTOR              = 0x0000'0000;
constexpr u32 UNDEFINED_INSTR_VECTOR    = 0x0000'0004;
constexpr u32 SWI_VECTOR                = 0x0000'0008;
constexpr u32 IRQ_VECTOR                = 0x0000'0018;

/// @brief Represent an undecoded ARM/THUMB instruction in the pipeline.
struct PrefetchedInstruction
{
    u32 Instruction;
    u32 PC;
};

/// @brief CPU operating mode, determines which register bank is in use.
enum class OperatingMode : u32
{
    User        = 0b10000,
    FIQ         = 0b10001,
    IRQ         = 0b10010,
    Supervisor  = 0b10011,
    Abort       = 0b10111,
    Undefined   = 0b11011,
    System      = 0b11111
};

/// @brief CPU operating state, determines whether to execute ARM or THUMB instructions.
enum class OperatingState : u32
{
    ARM     = 0,
    THUMB   = 1
};
}  // namespace cpu
