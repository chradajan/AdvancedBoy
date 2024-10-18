#pragma once

#include <array>
#include <cstddef>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <vector>
#include <GBA/include/APU/Constants.hpp>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>
#include <GBA/include/Utilities/Types.hpp>

namespace debug
{
///---------------------------------------------------------------------------------------------------------------------------------
/// Generic
///---------------------------------------------------------------------------------------------------------------------------------

struct DebugMemAccess
{
    std::span<const std::byte> memoryBlock;
    u32 minAddr;
    Page page;
    std::function<u32(u32)> AddrToIndex;
};

///---------------------------------------------------------------------------------------------------------------------------------
/// CPU
///---------------------------------------------------------------------------------------------------------------------------------

struct Mnemonic
{
    std::string op;
    std::string cond;
    std::string args;
    std::optional<i32> branchOffset;
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
    u8 mode;
};

struct CpuDebugInfo
{
    DebugMemAccess pcMem;
    DebugMemAccess spMem;
    RegState regState;
    u32 nextAddrToExecute;
};

///---------------------------------------------------------------------------------------------------------------------------------
/// PPU
///---------------------------------------------------------------------------------------------------------------------------------

struct BackgroundDebugInfo
{
    // Image
    std::array<u32, 1024 * 1024> buffer;
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

struct Sprite
{
    bool enabled;

    // Image
    std::array<u32, 128 * 128> buffer;
    u8 width;
    u8 height;

    // Location
    u16 x;
    u8 y;
    u16 tileIndex;
    u8 oamIndex;

    // Effects
    std::string gxfMode;
    bool mosaic;
    u8 palette;
    u8 priority;

    // Regular sprite
    bool regular;
    bool horizontalFlip;
    bool verticalFlip;

    // Affine sprite
    bool doubleSize;
    u8 parameterIndex;
    float pa;
    float pb;
    float pc;
    float pd;
};

using SpriteDebugInfo = std::array<Sprite, 128>;
}  // namespace debug
