#pragma once

#include <array>
#include <cstddef>
#include <GBA/include/Types.hpp>

/// @brief Manager of interrupt, waitstate, and power-down control registers.
class SystemControl
{
public:
    /// @brief Read an address mapped to system control registers.
    /// @param addr Address of system control register(s).
    /// @param length Memory access size of the read.
    /// @return Number of cycles taken to read, value of the requested register(s), and whether it was an open-bus read.
    MemReadData ReadReg(Address addr, AccessSize length);

    /// @brief Write to an address mapped to system control registers.
    /// @param addr Address of system control register(s).
    /// @param val Value to write to register(s).
    /// @param length Memory access size of the write.
    /// @return Number of cycles taken to write.
    CpuCycles WriteReg(Address addr, u32 val, AccessSize length);

private:
    std::array<std::byte, 0x0C> interruptAndWaitcntRegisters_;
    std::array<std::byte, 0x04> postFlgAndHaltcntRegisters_;
    std::array<std::byte, 0x04> undocumentedRegisters_;
    std::array<std::byte, 0x04> memoryControlRegisters_;
};
