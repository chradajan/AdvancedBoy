#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <utility>
#include <vector>
#include <GBA/include/Cartridge/BackupMedia.hpp>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Utilities/Types.hpp>

namespace debug { class GameBoyAdvanceDebugger; }
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
    /// @param saveDir Path to directory to save backup media into.
    /// @param scheduler Reference to event scheduler to determine prefetched waitstate timing.
    /// @param systemControl Reference to system control to check waitstate settings.
    explicit GamePak(fs::path romPath, fs::path saveDir, EventScheduler& scheduler, SystemControl& systemControl);

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

    /// @brief Check if a read/write is accessing memory in EEPROM.
    /// @param addr Address being accessed.
    /// @return True if accessing EEPROM.
    bool EepromAccess(u32 addr) const { return containsEeprom_ && backupMedia_->IsBackupMediaAccess(addr); }

    /// @brief Set the index of the next value to be read from EEPROM.
    /// @param index Index that will be read from.
    /// @param indexSize Number of bits in index.
    /// @return Number of cycles taken to set the index.
    int SetEepromIndex(u16 index, u8 indexSize);

    /// @brief Read a DWord from EEPROM at the previously set index.
    /// @return Value from EEPROM and number of cycles taken to read.
    std::pair<u64, int> ReadEepromDWord();

    /// @brief Write a DWord into EEPROM.
    /// @param index Index to write to.
    /// @param indexSize Number of bits in index.
    /// @param val Value to write to EEPROM.
    /// @return Number of cycles taken to write.
    int WriteEepromDWord(u16 index, u8 indexSize, u64 val);

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
    bool containsEeprom_;

    // Prefetch buffer
    u32 nextSequentialAddr_;
    u64 lastReadCompletionCycle_;
    int prefetchedWaitStates_;

    // External components
    EventScheduler& scheduler_;
    SystemControl& systemControl_;

    // Debug
    friend class debug::GameBoyAdvanceDebugger;
};
}  // namespace cartridge
