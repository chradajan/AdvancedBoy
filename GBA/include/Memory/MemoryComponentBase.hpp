#pragma once

#include <GBA/include/Types.hpp>

/// @brief Abstract base class for any component that manages memory-mapped blocks of memory.
class MemoryComponentBase
{
public:
    /// @brief Virtual destructor.
    virtual ~MemoryComponentBase() = default;

    /// @brief Read an address connected to the bus.
    /// @param addr Address to read from.
    /// @param length Memory access size of the read.
    /// @return Read data structure containing how many cycles the read took, the value returned from the read, and whether the read
    ///         operation resulted in an open-bus read.
    virtual MemReadData ReadMem(Address addr, AccessSize length) = 0;

    /// @brief Write to an address connected to the bus.
    /// @param addr Address to write to.
    /// @param val Value to write into memory.
    /// @param length Memory access size of the write.
    /// @return Number of CPU cycles taken to perform the write operation.
    virtual CpuCycles WriteMem(Address addr, u32 val, AccessSize length) = 0;
};