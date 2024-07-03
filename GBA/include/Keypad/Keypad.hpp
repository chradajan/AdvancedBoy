#pragma once

#include <array>
#include <cstddef>
#include <GBA/include/Memory/RegisterComponentBase.hpp>
#include <GBA/include/Types.hpp>

/// @brief GBA controller manager.
class Keypad : public RegisterComponentBase
{
public:
    /// @brief Read a Keypad register.
    MemReadData ReadReg(Address addr, AccessSize length) override;

    /// @brief Write a Keypad register.
    CpuCycles WriteReg(Address addr, u32 val, AccessSize length) override;

private:
    std::array<std::byte, 0x04> registers_;
};
