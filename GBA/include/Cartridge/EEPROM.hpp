#pragma once

#include <filesystem>
#include <GBA/include/Cartridge/BackupMedia.hpp>
#include <GBA/include/Types.hpp>

namespace fs = std::filesystem;

namespace cartridge
{
class EEPROM final : public virtual BackupMedia
{
public:
    EEPROM() = delete;
    EEPROM(EEPROM const&) = delete;
    EEPROM& operator=(EEPROM const&) = delete;
    EEPROM(EEPROM&&) = delete;
    EEPROM& operator=(EEPROM&&) = delete;

    /// @brief Initialize EEPROM backup media.
    /// @param savePath Path to file to save to.
    /// @param largeCart Whether the GamePak ROM is more than 16MB.
    explicit EEPROM(fs::path savePath, bool largeCart);

    /// @brief Check if a memory read/write is trying to access EEPROM.
    /// @param addr Address being read or written.
    /// @return Whether memory access is accessing EEPROM.
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
    bool largeCart_;
};
}  // namespace cartridge
