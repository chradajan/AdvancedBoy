#pragma once

#include <array>
#include <cstddef>
#include <GBA/include/Memory/MemoryComponentBase.hpp>
#include <GBA/include/Types.hpp>

/// @brief Manager for loading and reading/writing BIOS ROM.
class BIOSManager : public MemoryComponentBase
{
public:
    /// @brief Read BIOS memory.
    MemReadData ReadMem(Address addr, AccessSize length) override;

    /// @brief Write BIOS memory.
    CpuCycles WriteMem(Address addr, u32 val, AccessSize length) override;

private:
    std::array<std::byte, 16 * KiB> biosROM_;
};
