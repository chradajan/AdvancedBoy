#pragma once

#include <cstddef>
#include <vector>
#include <GBA/include/Types.hpp>

namespace cartridge
{
/// @brief Manager of GamePak ROM and backup memory.
class GamePak
{
public:
    /// @brief Read an address in GamePak memory.
    /// @param addr Address to read from.
    /// @param length Memory access size of the read.
    /// @return Number of cycles taken to read, value returned from the read, and whether it was an open-bus read.
    MemReadData ReadMem(u32 addr, AccessSize length);

    /// @brief Write to an address in GamePak memory.
    /// @param addr Address to write to.
    /// @param val Value to write.
    /// @param length Memory access size of the write.
    /// @return Number of cycles taken to write.
    int WriteMem(u32 addr, u32 val, AccessSize length);

    /// @brief Read an address in GamePak memory when no GamePak is currently loaded.
    /// @param addr Address to read from.
    /// @param length Memory access size of the read.
    /// @return Number of cycles taken to read, value returned from the read, and whether it was an open-bus read.
    static MemReadData ReadUnloadedGamePakMem(u32 addr, AccessSize length);

private:
    std::vector<std::byte> ROM_;
};
}  // namespace cartridge
