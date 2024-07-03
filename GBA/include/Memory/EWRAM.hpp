#pragma once

#include <array>
#include <cstddef>
#include <GBA/include/Memory/MemoryComponentBase.hpp>
#include <GBA/include/Types.hpp>

/// @brief External (slow) WRAM.
class EWRAM : public MemoryComponentBase
{
public:
    /// @brief Read EWRAM memory.
    MemReadData ReadMem(Address addr, AccessSize length) override;

    /// @brief Write EWRAM memory.
    CpuCycles WriteMem(Address addr, u32 val, AccessSize length) override;

private:
    std::array<std::byte, 256 * KiB> wram_;
};
