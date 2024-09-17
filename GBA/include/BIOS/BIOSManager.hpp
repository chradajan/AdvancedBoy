#pragma once

#include <array>
#include <cstddef>
#include <filesystem>
#include <span>
#include <GBA/include/Utilities/Functor.hpp>
#include <GBA/include/Utilities/Types.hpp>

namespace cpu { class ARM7TDMI; }
namespace debug { class GameBoyAdvanceDebugger; }
namespace fs = std::filesystem;

/// @brief Manager for loading and reading/writing BIOS ROM.
class BIOSManager
{
    using GetPCCallback = MemberFunctor<u32 (cpu::ARM7TDMI::*)()>;

public:
    BIOSManager() = delete;
    BIOSManager(BIOSManager const&) = delete;
    BIOSManager& operator=(BIOSManager const&) = delete;
    BIOSManager(BIOSManager&&) = delete;
    BIOSManager& operator=(BIOSManager&&) = delete;

    /// @brief Initialize the BIOS manager.
    /// @param biosPath Path to BIOS ROM.
    /// @param getPC Callback function to read the current PC value that code is executing from.
    explicit BIOSManager(fs::path biosPath, GetPCCallback getPC);

    /// @brief Read an address in BIOS memory.
    /// @param addr Address to read from.
    /// @param length Memory access size of the read.
    /// @return Number of cycles taken to read, value returned from the read, and whether it was an open-bus read.
    MemReadData ReadMem(u32 addr, AccessSize length);

    /// @brief Write to an address in BIOS memory. Has no effect.
    int WriteMem(u32, u32, AccessSize) const { return 1; }

    /// @brief Check if BIOS was successfully loaded into memory.
    /// @return True if BIOS is loaded.
    bool BiosLoaded() const { return biosLoaded_; }

private:
    GetPCCallback GetPC;
    std::array<std::byte, 16 * KiB> biosROM_;
    u32 lastSuccessfulFetch_;
    bool biosLoaded_;

    // Debug
    friend class debug::GameBoyAdvanceDebugger;
};
