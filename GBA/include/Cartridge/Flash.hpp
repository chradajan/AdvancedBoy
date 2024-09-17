#pragma once

#include <array>
#include <cstddef>
#include <filesystem>
#include <vector>
#include <GBA/include/Cartridge/BackupMedia.hpp>
#include <GBA/include/Utilities/Types.hpp>

namespace fs = std::filesystem;
class SystemControl;

namespace cartridge
{
class Flash final : public virtual BackupMedia
{
public:
    Flash() = delete;
    Flash(Flash const&) = delete;
    Flash& operator=(Flash const&) = delete;
    Flash(Flash&&) = delete;
    Flash& operator=(Flash&&) = delete;

    /// @brief Initialize Flash backup media.
    /// @param savePath Path to file to save to.
    /// @param largeFlash True for 128k, false for 64k.
    /// @param systemControl Reference to system control to access wait state timing.
    explicit Flash(fs::path savePath, bool largeFlash, SystemControl& systemControl);

    /// @brief Check if a memory read/write is trying to access Flash.
    /// @param addr Address being read or written.
    /// @return Whether memory access is accessing Flash.
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
    enum class FlashCommand : u8
    {
        CMD_SEQ_START           = 0xAA,
        CMD_SEQ_AWAIT           = 0x55,
        ENTER_CHIP_ID_MODE      = 0x90,
        EXIT_CHIP_ID_MODE       = 0xF0,
        PREPARE_ERASE           = 0x80,
        ERASE_ALL               = 0x10,
        ERASE_4K_SECTOR         = 0x30,
        WRITE_DATA              = 0xA0,
        SET_BANK                = 0xB0,
    };

    enum class FlashState
    {
        READY,
        CMD_SEQ_STARTED,
        CMD_SEQ_AWAITING_CMD,

        ERASE_SEQ_READY,
        ERASE_SEQ_STARTED,
        ERASE_SEQ_AWAITING_CMD,

        AWAITING_WRITE_DATA,
        AWAITING_BANK
    };

    static constexpr u32 CMD_ADDR_1 = 0x0E00'5555;
    static constexpr u32 CMD_ADDR_2 = 0x0E00'2AAA;

    /// @brief Update state based on command sent to flash.
    /// @param cmd Command sent to flash.
    void ProcessCommand(FlashCommand cmd);

    std::vector<std::array<std::byte, 64 * KiB>> flash_;
    u8 bank_;
    FlashState state_;
    bool chipIdMode_;
    bool const bankedFlash_;

    // External components
    SystemControl& systemControl_;
};
}  // namespace cartridge
