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
    MemReadData8 Read8(Address addr) override;
    MemReadData16 Read16(Address addr) override;
    MemReadData32 Read32(Address addr) override;

    CpuCycles Write8(Address addr, u8 val) override;
    CpuCycles Write16(Address addr, u16 val) override;
    CpuCycles Write32(Address addr, u32 val) override;

    MemReadData8 ReadReg8(Address addr) override;
    MemReadData16 ReadReg16(Address addr) override;
    MemReadData32 ReadReg32(Address addr) override;

    CpuCycles WriteReg8(Address addr, u8 val) override;
    CpuCycles WriteReg16(Address addr, u16 val) override;
    CpuCycles WriteReg32(Address addr, u32 val) override;

private:
    // Memory
    std::array<std::byte,  1 * KiB> PRAM_;
    std::array<std::byte,  1 * KiB> OAM_;
    std::array<std::byte, 96 * KiB> VRAM_;

    // Registers
    std::array<std::byte, 0x58> registers_;
};
}  // namespace graphics
