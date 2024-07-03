#pragma once

#include <array>
#include <cstddef>
#include <GBA/include/Memory/RegisterComponentBase.hpp>
#include <GBA/include/Types.hpp>

/// @brief Manager of interrupt, waitstate, and power-down control registers.
class SystemControl : public RegisterComponentBase
{
public:
    /// @brief Read a System Control register.
    MemReadData ReadReg(Address addr, AccessSize length) override;

    /// @brief Write a System Control register.
    CpuCycles WriteReg(Address addr, u32 val, AccessSize length) override;

private:
    std::array<std::byte, 0x0C> interruptAndWaitcntRegisters_;
    std::array<std::byte, 0x04> postFlgAndHaltcntRegisters_;
    std::array<std::byte, 0x04> undocumentedRegisters_;
    std::array<std::byte, 0x04> memoryControlRegisters_;
};
