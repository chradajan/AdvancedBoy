#pragma once

#include <array>
#include <cstddef>
#include <GBA/include/Types.hpp>

/// @brief Possible waitstate regions to use for GamePak access.
enum class WaitStateRegion
{
    ZERO,
    ONE,
    TWO,
    SRAM
};

/// @brief Manager of interrupt, waitstate, and power-down control registers.
class SystemControl
{
public:
    /// @brief Initialize the system control registers.
    SystemControl();

    SystemControl(SystemControl const&) = delete;
    SystemControl& operator=(SystemControl const&) = delete;
    SystemControl(SystemControl&&) = delete;
    SystemControl& operator=(SystemControl&&) = delete;

    /// @brief Read an address mapped to system control registers.
    /// @param addr Address of system control register(s).
    /// @param length Memory access size of the read.
    /// @return Number of cycles taken to read, value of the requested register(s), and whether it was an open-bus read.
    MemReadData ReadReg(u32 addr, AccessSize length);

    /// @brief Write to an address mapped to system control registers.
    /// @param addr Address of system control register(s).
    /// @param val Value to write to register(s).
    /// @param length Memory access size of the write.
    /// @return Number of cycles taken to write.
    int WriteReg(u32 addr, u32 val, AccessSize length);

    /// @brief Check if the CPU is halted.
    /// @return Whether the CPU is currently halted.
    bool Halted() const { return false; }

    /// @brief Check if an IRQ is pending. True if IF & IE != 0, and IME is enabled.
    /// @return Whether an IRQ is pending.
    bool IrqPending() const { return false; }

private:
    std::array<std::byte, 0x0C> interruptAndWaitcntRegisters_;
    std::array<std::byte, 0x04> postFlgAndHaltcntRegisters_;
    std::array<std::byte, 0x04> undocumentedRegisters_;
    std::array<std::byte, 0x04> memoryControlRegisters_;
};
