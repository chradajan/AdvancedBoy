#pragma once

#include <array>
#include <cstddef>
#include <GBA/include/Memory/MemoryComponentBase.hpp>
#include <GBA/include/Types.hpp>

/// @brief External (slow) WRAM.
class IWRAM : public MemoryComponentBase
{
public:
    /// @brief Read IWRAM memory.
    MemReadData ReadMem(Address addr, AccessSize length) override;

    /// @brief Write IWRAM memory.
    CpuCycles WriteMem(Address addr, u32 val, AccessSize length) override;

private:
    std::array<std::byte, 32 * KiB> wram_;
};
