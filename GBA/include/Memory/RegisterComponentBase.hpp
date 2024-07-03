#pragma once

#include <GBA/include/Types.hpp>

/// @brief Abstract base class for any component that manages memory-mapped registers.
class RegisterComponentBase
{
public:
    /// @brief Virtual destructor.
    virtual ~RegisterComponentBase() = default;

    /// @brief Read a memory-mapped register.
    /// @param addr Address of register to read.
    /// @param length Memory access size of the read.
    /// @return Read data structure containing how many cycles the read took, the value returned from the read, and whether the read
    ///         operation resulted in an open-bus read.
    virtual MemReadData ReadReg(Address addr, AccessSize length) = 0;

    /// @brief Write to a memory-mapped register.
    /// @param addr Address of register to read.
    /// @param val Value to write to the register.
    /// @param length Memory access size of the write.
    /// @return Number of CPU cycles taken to perform the write operation.
    virtual CpuCycles WriteReg(Address addr, u32 val, AccessSize length) = 0;
};
