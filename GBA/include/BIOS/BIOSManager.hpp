#pragma once

#include <array>
#include <cstddef>
#include <GBA/include/Memory/MemoryComponentBase.hpp>
#include <GBA/include/Types.hpp>

namespace bios
{
/// @brief Manager for loading and reading/writing BIOS ROM.
class BIOSManager : public MemoryComponentBase
{
public:
    MemReadData8 Read8(Address addr) override;
    MemReadData16 Read16(Address addr) override;
    MemReadData32 Read32(Address addr) override;

    CpuCycles Write8(Address, u8) override;
    CpuCycles Write16(Address, u16) override;
    CpuCycles Write32(Address, u32) override;

private:
    std::array<std::byte, 16 * KiB> biosROM_;
};
}  // namespace bios
