#pragma once

#include <array>
#include <cstddef>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/Types/Types.hpp>
#include <GBA/include/Utilities/CircularBuffer.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>

namespace debug
{
///---------------------------------------------------------------------------------------------------------------------------------
/// CPU
///---------------------------------------------------------------------------------------------------------------------------------

namespace cpu
{
struct DisassembledInstruction
{
    bool armInstruction;
    u32 undecodedInstruction;
    u32 addr;

    std::string op;
    std::string cond;
    std::string args;
};

struct RegState
{
    std::array<u32, 16> registers;
    u32 cpsr;
    std::optional<u32> spsr;

    bool negative;
    bool zero;
    bool carry;
    bool overflow;

    bool irqDisable;
    bool fiqDisable;
    bool thumbState;
    ::cpu::OperatingMode mode;
};

struct CpuDebugInfo
{
    CircularBuffer<DisassembledInstruction, 10> prevInstructions;
    std::optional<DisassembledInstruction> currInstruction;
    CircularBuffer<DisassembledInstruction, 20> nextInstructions;
    RegState regState;
};

struct CpuFastMemAccess
{
    std::span<const std::byte> memoryBlock;
    std::function<u32(u32)> AddrToIndex;
};
}  // namespace cpu

///---------------------------------------------------------------------------------------------------------------------------------
/// PPU
///---------------------------------------------------------------------------------------------------------------------------------

namespace graphics
{
struct BackgroundDebugInfo
{
    // Image
    std::vector<u16> buffer;
    u16 width;
    u16 height;

    // Info
    u8 priority;
    u32 mapBaseAddr;
    u32 tileBaseAddr;

    // Regular
    bool regular;
    u16 xOffset;
    u16 yOffset;

    // Affine
    float refX;
    float refY;
    float pa;
    float pb;
    float pc;
    float pd;
};
}  // namespace graphics
}  // namespace debug
