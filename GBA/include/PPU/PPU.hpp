#pragma once

#include <array>
#include <cstddef>
#include <GBA/include/Memory/MemoryComponentBase.hpp>
#include <GBA/include/Memory/RegisterComponentBase.hpp>
#include <GBA/include/Types.hpp>

namespace graphics
{
/// @brief Pixel Processing Unit.
class PPU : public MemoryComponentBase, public RegisterComponentBase
{
public:
    /// @brief Read PPU memory.
    MemReadData ReadMem(Address addr, AccessSize length) override;

    /// @brief Write PPU memory.
    CpuCycles WriteMem(Address addr, u32 val, AccessSize length) override;

    /// @brief Read a PPU register.
    MemReadData ReadReg(Address addr, AccessSize length) override;

    /// @brief Write a PPU register.
    CpuCycles WriteReg(Address addr, u32 val, AccessSize length) override;

private:
    // Memory
    std::array<std::byte,  1 * KiB> PRAM_;
    std::array<std::byte,  1 * KiB> OAM_;
    std::array<std::byte, 96 * KiB> VRAM_;

    // Registers
    std::array<std::byte, 0x58> registers_;
};
}  // namespace graphics
