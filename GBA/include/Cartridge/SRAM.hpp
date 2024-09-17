#pragma once

#include <array>
#include <cstddef>
#include <filesystem>
#include <GBA/include/Cartridge/BackupMedia.hpp>
#include <GBA/include/Utilities/Types.hpp>

namespace fs = std::filesystem;
class SystemControl;

namespace cartridge
{
class SRAM final : public virtual BackupMedia
{
public:
    SRAM() = delete;
    SRAM(SRAM const&) = delete;
    SRAM& operator=(SRAM const&) = delete;
    SRAM(SRAM&&) = delete;
    SRAM& operator=(SRAM&&) = delete;

    /// @brief Initialize SRAM backup media.
    /// @param savePath Path to file to save to.
    /// @param systemControl Reference to system control to access wait state timing.
    explicit SRAM(fs::path savePath, SystemControl& systemControl);

    /// @brief Check if a memory read/write is trying to access SRAM.
    /// @param addr Address being read or written.
    /// @return Whether memory access is accessing SRAM.
    bool IsBackupMediaAccess(u32 addr) const override;

    /// @brief Read an address in backup media address space.
    /// @param addr Address to read from.
    /// @param length Memory access size of the read.
    /// @return Number of cycles taken to read, value returned from the read, and whether it was an open-bus read.
    MemReadData ReadMem(u32 addr, AccessSize length) override;

    /// @brief Write to an address in backup media address space.
    /// @param addr Address to write to.
    /// @param val Value to write.
    /// @param length Memory access size of the write.
    /// @return Number of cycles taken to write.
    int WriteMem(u32 addr, u32 val, AccessSize length) override;

    /// @brief Save backup media to the saved path.
    void Save() const override;

private:
    std::array<std::byte, 32 * KiB> sram_;

    // External components
    SystemControl& systemControl_;
};
}  // namespace cartridge
