#pragma once

#include <array>
#include <cstddef>
#include <GBA/include/Types.hpp>

namespace timers
{
/// @brief Manager class for system timers.
class TimerManager
{
public:
    /// @brief Read an address mapped to timer registers.
    /// @param addr Address of timer register(s).
    /// @param length Memory access size of the read.
    /// @return Number of cycles taken to read, value of the requested register(s), and whether it was an open-bus read.
    MemReadData ReadReg(Address addr, AccessSize length);

    /// @brief Write to an address mapped to timer registers.
    /// @param addr Address of timer register(s).
    /// @param val Value to write to register(s).
    /// @param length Memory access size of the write.
    /// @return Number of cycles taken to write.
    CpuCycles WriteReg(Address addr, u32 val, AccessSize length);

private:
    std::array<std::byte, 0x10> registers_;
};
}  // namespace timers
