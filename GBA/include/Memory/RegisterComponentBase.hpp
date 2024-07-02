#pragma once

#include <GBA/include/Types.hpp>

/// @brief Abstract base class for any component that manages memory-mapped registers.
class RegisterComponentBase
{
public:
    virtual ~RegisterComponentBase() = default;

    virtual MemReadData8 ReadReg8(Address addr) = 0;
    virtual MemReadData16 ReadReg16(Address addr) = 0;
    virtual MemReadData32 ReadReg32(Address addr) = 0;

    virtual CpuCycles WriteReg8(Address addr, u8 val) = 0;
    virtual CpuCycles WriteReg16(Address addr, u16 val) = 0;
    virtual CpuCycles WriteReg32(Address addr, u32 val) = 0;
};
