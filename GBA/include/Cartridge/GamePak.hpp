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
    /// @brief Read GamePak memory.
    MemReadData ReadMem(Address addr, AccessSize length) override;

    /// @brief Write GamePak memory.
    CpuCycles WriteMem(Address addr, u32 val, AccessSize length) override;

private:
    std::vector<std::byte> ROM_;
};
}  // namespace cartridge
