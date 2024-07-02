#pragma once

#include <cstddef>
#include <vector>
#include <GBA/include/Memory/MemoryComponentBase.hpp>
#include <GBA/include/Types.hpp>

namespace cartridge
{
/// @brief Manager of GamePak ROM and backup memory.
class GamePak : public MemoryComponentBase
{
public:
    MemReadData8 Read8(Address addr) override;
    MemReadData16 Read16(Address addr) override;
    MemReadData32 Read32(Address addr) override;

    CpuCycles Write8(Address, u8) override;
    CpuCycles Write16(Address, u16) override;
    CpuCycles Write32(Address, u32) override;

private:
    std::vector<std::byte> ROM_;
};
}  // namespace cartridge
