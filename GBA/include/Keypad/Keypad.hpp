#pragma once

#include <array>
#include <cstddef>
#include <fstream>
#include <GBA/include/Keypad/Registers.hpp>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Utilities/Types.hpp>

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

    /// @brief Update the KEYINPUT register based on current user input.
    /// @param keyinput KEYINPUT value.
    void UpdateKeypad(KEYINPUT keyinput);

    /// @brief Read an address mapped to Keypad registers.
    /// @param addr Address of Keypad register(s).
    /// @param length Memory access size of the read.
    /// @return Number of cycles taken to read, value of the requested register(s), and whether it was an open-bus read.
    MemReadData ReadReg(u32 addr, AccessSize length);

    /// @brief Write to an address mapped to Keypad registers.
    /// @param addr Address of Keypad register(s).
    /// @param val Value to write to register(s).
    /// @param length Memory access size of the write.
    /// @return Number of cycles taken to write.
    int WriteReg(u32 addr, u32 val, AccessSize length);

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Save States
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Write data to save state file.
    /// @param saveState Save state stream to write to.
    void Serialize(std::ofstream& saveState) const;

    /// @brief Load data from save state file.
    /// @param saveState Save state stream to read from.
    void Deserialize(std::ifstream& saveState);

private:
    /// @brief Check if Gamepad IRQ should be requested.
    void CheckKeypadIRQ();

    std::array<std::byte, 0x04> registers_;

    // External components
    SystemControl& systemControl_;
};
