#pragma once

#include <GBA/include/Types.hpp>

/// @brief Abstract base class for any component that manages memory-mapped blocks of memory.
class MemoryComponentBase
{
public:
    virtual ~MemoryComponentBase() = default;

    virtual MemReadData8 Read8(Address addr) = 0;
    virtual MemReadData16 Read16(Address addr) = 0;
    virtual MemReadData32 Read32(Address addr) = 0;

    virtual CpuCycles Write8(Address addr, u8 val) = 0;
    virtual CpuCycles Write16(Address addr, u16 val) = 0;
    virtual CpuCycles Write32(Address addr, u32 val) = 0;
};