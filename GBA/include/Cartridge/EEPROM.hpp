#pragma once

#include <filesystem>
#include <utility>
#include <vector>
#include <GBA/include/Cartridge/BackupMedia.hpp>
#include <GBA/include/Types/Types.hpp>

namespace fs = std::filesystem;
class SystemControl;

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
    /// @param systemControl Reference to system control to access wait state timing.
    explicit EEPROM(fs::path savePath, bool largeCart, SystemControl& systemControl);

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

    /// @brief Set the index of the next value to be read from EEPROM.
    /// @param index Index that will be read from.
    /// @param indexSize Number of bits in index.
    /// @return Number of cycles taken to set the index.
    int SetIndex(u16 index, u8 indexSize);

    /// @brief Read a DWord from EEPROM at the previously set index.
    /// @return Value from EEPROM and number of cycles taken to read.
    std::pair<u64, int> ReadDWord();

    /// @brief Write a DWord into EEPROM.
    /// @param index Index to write to.
    /// @param indexSize Number of bits in index.
    /// @param val Value to write to EEPROM.
    /// @return Number of cycles taken to write.
    int WriteDWord(u16 index, u8 indexSize, u64 val);

    /// @brief Save backup media to the saved path.
    void Save() const override;

private:
    std::vector<u64> eeprom_;
    u16 readIndex_;
    bool const largeCart_;

    // External components
    SystemControl& systemControl_;
};
}  // namespace cartridge
