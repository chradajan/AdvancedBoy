#pragma once

#include <array>
#include <cstddef>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Types.hpp>

/// @brief GBA controller manager.
class Keypad
{
public:
    Keypad() = delete;
    Keypad(Keypad const&) = delete;
    Keypad& operator=(Keypad const&) = delete;
    Keypad(Keypad&&) = delete;
    Keypad& operator=(Keypad&&) = delete;

    /// @brief Initialize the keypad.
    /// @param systemControl Reference to system control to post keypad interrupts to.
    explicit Keypad(SystemControl& systemControl);

    /// @brief Read an address mapped to Keypad registers.
    /// @param addr Address of Keypad register(s).
    /// @param length Memory access size of the read.
    /// @return Number of cycles taken to read, value of the requested register(s), and whether it was an open-bus read.
    MemReadData ReadReg(Address addr, AccessSize length);

    /// @brief Write to an address mapped to Keypad registers.
    /// @param addr Address of Keypad register(s).
    /// @param val Value to write to register(s).
    /// @param length Memory access size of the write.
    /// @return Number of cycles taken to write.
    CpuCycles WriteReg(Address addr, u32 val, AccessSize length);

private:
    std::array<std::byte, 0x04> registers_;

    // External components
    SystemControl& systemControl_;
};
