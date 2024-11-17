#include <GBA/include/Cartridge/GamePak.hpp>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <GBA/include/Cartridge/EEPROM.hpp>
#include <GBA/include/Cartridge/Flash.hpp>
#include <GBA/include/Cartridge/SRAM.hpp>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>
#include <GBA/include/Utilities/Types.hpp>

namespace cartridge
{
GamePak::GamePak(fs::path romPath, fs::path saveDir, EventScheduler& scheduler, SystemControl& systemControl) :
    scheduler_(scheduler),
    systemControl_(systemControl)
{
    title_ = "";
    gamePakLoaded_ = false;
    containsEeprom_ = false;

    nextSequentialAddr_ = U32_MAX;
    lastReadCompletionCycle_ = 0;
    prefetchedWaitStates_ = 0;
    size_t fileSizeInBytes = fs::file_size(romPath);

    if (romPath.empty() || !fs::exists(romPath) || !fs::is_regular_file(romPath) || (fileSizeInBytes > (32 * MiB)))
    {
        return;
    }

    ROM_.resize(fileSizeInBytes);
    std::ifstream rom(romPath, std::ios::binary);

    if (rom.fail())
    {
        return;
    }

    rom.read(reinterpret_cast<char*>(ROM_.data()), fileSizeInBytes);

    if (!ValidHeader())
    {
        return;
    }

    std::stringstream titleStream;

    for (u8 i = 0xA0; i < 0xAC; ++i)
    {
        char c = static_cast<char>(ROM_[i]);

        if (c == 0)
        {
            continue;
        }

        titleStream << c;
    }

    title_ = titleStream.str();

    // Determine backup type and load save file if present
    auto backupType = DetectBackupType();
    std::string saveFileName = title_;
    std::replace(saveFileName.begin(), saveFileName.end(), ' ', '_');
    savePath_ = saveDir / saveFileName;
    savePath_ += ".sav";

    if (savePath_.filename().empty())
    {
        savePath_.replace_filename(romPath.filename());
    }

    switch (backupType)
    {
        case BackupType::SRAM:
            backupMedia_ = std::make_unique<SRAM>(savePath_, systemControl);
            break;
        case BackupType::FLASH_64:
            backupMedia_ = std::make_unique<Flash>(savePath_, false, systemControl);
            break;
        case BackupType::FLASH_128:
            backupMedia_ = std::make_unique<Flash>(savePath_, true, systemControl);
            break;
        case BackupType::EEPROM:
            backupMedia_ = std::make_unique<EEPROM>(savePath_, ROM_.size() > (16 * MiB), systemControl);
            containsEeprom_ = true;
            break;
        default:
            backupMedia_ = nullptr;
            break;
    }

    gamePakLoaded_ = true;
}

MemReadData GamePak::ReadMem(u32 addr, AccessSize length)
{
    if (backupMedia_ && backupMedia_->IsBackupMediaAccess(addr))
    {
        return backupMedia_->ReadMem(addr, length);
    }

    WaitStateRegion region;
    u8 page = (addr & 0x0F00'0000) >> 24;

    switch (page)
    {
        case 0x08 ... 0x09:
            region = WaitStateRegion::ZERO;
            break;
        case 0x0A ... 0x0B:
            region = WaitStateRegion::ONE;
            addr -= (32 * MiB);
            break;
        case 0x0C ... 0x0D:
            region = WaitStateRegion::TWO;
            addr -= 2 * (32 * MiB);
            break;
        default:
            return {1, 0, true};
    }

    if ((addr - GAMEPAK_ROM_ADDR_MIN) >= ROM_.size())
    {
        return {1, 0, true};
    }

    int cycles = 1;
    bool sequential = addr == nextSequentialAddr_;
    u64 currentCycle = scheduler_.GetTotalElapsedCycles();
    int waitStates = systemControl_.WaitStates(region, sequential, length);

    if (systemControl_.GamePakPrefetchEnabled())
    {
        if (sequential)
        {
            int maxPrefetchedWaitStates = 8 * systemControl_.WaitStates(region, true, AccessSize::HALFWORD);
            prefetchedWaitStates_ = std::min(prefetchedWaitStates_ + (currentCycle - lastReadCompletionCycle_),
                                             static_cast<u64>(maxPrefetchedWaitStates));

            if (prefetchedWaitStates_ >= waitStates)
            {
                prefetchedWaitStates_ -= waitStates;
                waitStates = 0;
            }
            else
            {
                waitStates -= prefetchedWaitStates_;
                prefetchedWaitStates_ = 0;
            }
        }
        else
        {
            prefetchedWaitStates_ = 0;
        }
    }
    else
    {
        prefetchedWaitStates_ = 0;
    }

    nextSequentialAddr_ = addr + static_cast<u8>(length);
    cycles += waitStates;
    lastReadCompletionCycle_ = currentCycle + cycles;

    u32 val = ReadMemoryBlock(ROM_, addr, GAMEPAK_ROM_ADDR_MIN, length);
    return {cycles, val, false};
}

int GamePak::WriteMem(u32 addr, u32 val, AccessSize length)
{
    if (backupMedia_ && backupMedia_->IsBackupMediaAccess(addr))
    {
        return backupMedia_->WriteMem(addr, val, length);
    }

    return 1;
}

MemReadData GamePak::ReadUnloadedGamePakMem(u32 addr, AccessSize length)
{
    u32 val = 0;
    size_t count = static_cast<size_t>(length);
    u32 currAddr = addr + count - 1;

    while (count > 0)
    {
        val <<= 8;
        val |= ((currAddr / 2) & 0x0000'FFFF);
        --count;
    }

    return {1, val, false};
}

int GamePak::SetEepromIndex(u16 index, u8 indexSize)
{
    if (containsEeprom_)
    {
        return dynamic_cast<EEPROM*>(backupMedia_.get())->SetIndex(index, indexSize);
    }

    return 1;
}

std::pair<u64, int> GamePak::ReadEepromDWord()
{
    if (containsEeprom_)
    {
        return dynamic_cast<EEPROM*>(backupMedia_.get())->ReadDWord();
    }

    return {U64_MAX, 1};
}

int GamePak::WriteEepromDWord(u16 index, u8 indexSize, u64 val)
{
    if (containsEeprom_)
    {
        return dynamic_cast<EEPROM*>(backupMedia_.get())->WriteDWord(index, indexSize, val);
    }

    return 1;
}

void GamePak::Save() const
{
    if (backupMedia_)
    {
        backupMedia_->Save();
    }
}

void GamePak::Serialize(std::ofstream& saveState) const
{
    if (backupMedia_)
    {
        backupMedia_->Serialize(saveState);
    }

    SerializeTrivialType(nextSequentialAddr_);
    SerializeTrivialType(lastReadCompletionCycle_);
    SerializeTrivialType(prefetchedWaitStates_);
}

void GamePak::Deserialize(std::ifstream& saveState)
{
    if (backupMedia_)
    {
        backupMedia_->Deserialize(saveState);
    }

    DeserializeTrivialType(nextSequentialAddr_);
    DeserializeTrivialType(lastReadCompletionCycle_);
    DeserializeTrivialType(prefetchedWaitStates_);
}

bool GamePak::ValidHeader() const
{
    if (ROM_.size() < 192)
    {
        return false;
    }

    // Sum up the first 152 bytes of the compressed bitmap of the Nintendo logo in the header since these must always be the same
    // for any valid GBA ROM.
    u16 headerSum = 0;

    for (u8 i = 0x04; i < 0x9C; ++i)
    {
        headerSum += static_cast<u8>(ROM_[i]);
    }

    return headerSum == 0x4927;
}

BackupType GamePak::DetectBackupType() const
{
    for (size_t i = 0; (i + 11) < ROM_.size(); i += 4)
    {
        auto start = reinterpret_cast<const char*>(&ROM_[i]);
        std::string idString;
        idString.assign(start, 12);

        if (idString.starts_with("EEPROM_V"))
        {
            return BackupType::EEPROM;
        }
        else if (idString.starts_with("SRAM_V"))
        {
            return BackupType::SRAM;
        }
        else if (idString.starts_with("FLASH"))
        {
            if (idString.starts_with("FLASH_V") || idString.starts_with("FLASH512_V"))
            {
                return BackupType::FLASH_64;
            }
            else if (idString.starts_with("FLASH1M_V"))
            {
                return BackupType::FLASH_128;
            }
        }
    }

    return BackupType::NONE;
}
}  // namespace cartridge
