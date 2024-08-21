#pragma once

#include <bit>
#include <filesystem>
#include <GBA/include/Types.hpp>

namespace fs = std::filesystem;

namespace cartridge
{
enum class BackupType
{
    NONE,
    SRAM,
    EEPROM,
    FLASH_64,
    FLASH_128
};

/// @brief Abstract base class that SRAM, EEPROM, and Flash inherit from.
class BackupMedia
{
public:
    virtual ~BackupMedia() = default;

    /// @brief Check if a memory read/write is trying to access backup media.
    /// @param addr Address being read or written.
    /// @return Whether memory access is accessing backup media.
    virtual bool IsBackupMediaAccess(u32 addr) const = 0;

    /// @brief Read an address in backup media address space.
    /// @param addr Address to read from.
    /// @param length Memory access size of the read.
    /// @return Number of cycles taken to read, value returned from the read, and whether it was an open-bus read.
    virtual MemReadData ReadMem(u32 addr, AccessSize length) = 0;

    /// @brief Write to an address in backup media address space.
    /// @param addr Address to write to.
    /// @param val Value to write.
    /// @param length Memory access size of the write.
    /// @return Number of cycles taken to write.
    virtual int WriteMem(u32 addr, u32 val, AccessSize length) = 0;

    /// @brief Save backup media to the saved path.
    virtual void Save() const = 0;

protected:
    fs::path savePath_;
};
}  // namespace cartridge
