#pragma once

#include <array>
#include <cstddef>
#include <GBA/include/Types.hpp>

/// @brief Manager for loading and reading/writing BIOS ROM.
class BIOSManager
{
public:
    /// @brief Read an address in BIOS memory.
    /// @param addr Address to read from.
    /// @param length Memory access size of the read.
    /// @return Number of cycles taken to read, value returned from the read, and whether it was an open-bus read.
    MemReadData ReadMem(Address addr, AccessSize length);

    /// @brief Write to an address in BIOS memory.
    /// @param addr Address to write to.
    /// @param val Value to write.
    /// @param length Memory access size of the write.
    /// @return Number of cycles taken to write.
    CpuCycles WriteMem(Address addr, u32 val, AccessSize length);

private:
    std::array<std::byte, 16 * KiB> biosROM_;
};
