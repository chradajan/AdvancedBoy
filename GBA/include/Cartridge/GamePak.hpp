#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>
#include <GBA/include/Cartridge/BackupMedia.hpp>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Types.hpp>

namespace fs = std::filesystem;

namespace cartridge
{
/// @brief Manager of GamePak ROM and backup memory.
class GamePak
{
public:
    GamePak() = delete;
    GamePak(GamePak const&) = delete;
    GamePak& operator=(GamePak const&) = delete;
    GamePak(GamePak&&) = delete;
    GamePak& operator=(GamePak&&) = delete;

    /// @brief Load cartridge ROM into memory and load a save file if one exists.
    /// @param romPath Path to ROM. Looks for save file in the same directory.
    /// @param scheduler Reference to event scheduler to determine prefetched waitstate timing.
    /// @param systemControl Reference to system control to check waitstate settings.
    explicit GamePak(fs::path romPath, EventScheduler& scheduler, SystemControl& systemControl);

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

    /// @brief Check if GamePak was successfully loaded into memory.
    /// @return True if a GamePak is loaded.
    bool GamePakLoaded() const { return gamePakLoaded_; }

    /// @brief Get the title of the ROM currently running.
    /// @return Current ROM title.
    std::string GetTitle() const { return title_; }

    /// @brief TODO: Check if a read/write is accessing memory in EEPROM.
    /// @param addr Address being accessed.
    /// @return True if accessing EEPROM.
    bool EepromAccess(u32 addr) const { (void)addr; return false; }

    /// @brief Save backup media to disk.
    void Save() const;

private:
    /// @brief Determine the backup type used by a ROM.
    /// @return Backup type if one was detected.
    BackupType DetectBackupType() const;

    std::vector<std::byte> ROM_;
    std::unique_ptr<BackupMedia> backupMedia_;
    std::string title_;
    bool gamePakLoaded_;

    // External components
    EventScheduler& scheduler_;
    SystemControl& systemControl_;
};
}  // namespace cartridge
