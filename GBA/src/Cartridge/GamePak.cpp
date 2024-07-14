#include <GBA/include/Cartridge/GamePak.hpp>
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
#include <GBA/include/Types.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>

namespace cartridge
{
GamePak::GamePak(fs::path romPath, EventScheduler& scheduler, SystemControl& systemControl) :
    scheduler_(scheduler),
    systemControl_(systemControl)
{
    title_ = "";
    gamePakLoaded_ = false;

    if (romPath.empty() || !fs::exists(romPath) || !fs::is_regular_file(romPath))
    {
        return;
    }

    size_t fileSizeInBytes = fs::file_size(romPath);
    ROM_.resize(fileSizeInBytes);
    std::ifstream rom(romPath, std::ios::binary);

    if (rom.fail())
    {
        return;
    }

    rom.read(reinterpret_cast<char*>(ROM_.data()), fileSizeInBytes);

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

    // TODO: Implement backup media
    backupMedia_ = nullptr;

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

    // TODO: Calculate actual cycles based on waitstate region and prefetcher buffer settings
    (void)region;
    int cycles = 1;

    if ((addr - GAMEPAK_ROM_ADDR_MIN) >= ROM_.size())
    {
        return {1, 0, true};
    }

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
}  // namespace cartridge
