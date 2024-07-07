#include <GBA/include/GameBoyAdvance.hpp>
#include <array>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <GBA/include/APU/APU.hpp>
#include <GBA/include/BIOS/BIOSManager.hpp>
#include <GBA/include/Cartridge/GamePak.hpp>
#include <GBA/include/CPU/ARM7TDMI.hpp>
#include <GBA/include/DMA/DmaManager.hpp>
#include <GBA/include/Keypad/Keypad.hpp>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/PPU/PPU.hpp>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Timers/TimerManager.hpp>
#include <GBA/include/Types.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>

namespace
{
u32 ForceAlignAddress(u32 addr, AccessSize length)
{
    u32 alignment = static_cast<u32>(length) - 1;

    if ((addr & alignment) != 0)
    {
        addr &= ~alignment;
    }

    return addr;
}
}

GameBoyAdvance::GameBoyAdvance(fs::path biosPath) :
    apu_(scheduler_),
    biosMgr_(biosPath, {&cpu::ARM7TDMI::GetPC, cpu_}),
    cpu_({&ReadMem, *this}, {&WriteMem, *this}),
    dmaMgr_({&ReadMem, *this}, {&WriteMem, *this}, systemControl_),
    keypad_(systemControl_),
    ppu_(scheduler_, systemControl_),
    timerMgr_(scheduler_, systemControl_)
{
    EWRAM_.fill(std::byte{0});
    IWRAM_.fill(std::byte{0});
}

std::pair<u32, int> GameBoyAdvance::ReadMem(u32 addr, AccessSize length)
{
    addr = ForceAlignAddress(addr, length);
    auto page = Page::INVALID;
    MemReadData readData;

    if (addr < 0x1000'0000)
    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wnarrowing"
        page = Page{(addr & 0x0F00'0000) >> 24};
        #pragma GCC diagnostic pop
    }

    switch (page)
    {
        case Page::BIOS:
            readData = biosMgr_.ReadMem(addr, length);
            break;
        case Page::EWRAM:
            readData = ReadEWRAM(addr, length);
            break;
        case Page::IWRAM:
            readData = ReadIWRAM(addr, length);
            break;
        case Page::IO:
            readData = ReadIO(addr, length);
            break;
        case Page::PRAM:
            readData = ppu_.ReadPRAM(addr, length);
            break;
        case Page::VRAM:
            readData = ppu_.ReadVRAM(addr, length);
            break;
        case Page::OAM:
            readData = ppu_.ReadOAM(addr, length);
            break;
        case Page::GAMEPAK_MIN ... Page::GAMEPAK_MAX:
            readData = gamePak_ ? gamePak_->ReadMem(addr, length) : cartridge::GamePak::ReadUnloadedGamePakMem(addr, length);
            break;
        case Page::INVALID:
        default:
            readData = {1, 0, true};
            break;
    }

    if (readData.OpenBus)
    {
        // TODO
        readData.Value = 0;
        readData.Cycles = 1;
    }

    return {readData.Value, readData.Cycles};
}

int GameBoyAdvance::WriteMem(u32 addr, u32 val, AccessSize length)
{
    (void)val;

    addr = ForceAlignAddress(addr, length);
    auto page = Page::INVALID;
    int cycles = 1;

    if (addr < 0x1000'0000)
    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wnarrowing"
        page = Page{(addr & 0x0F00'0000) >> 24};
        #pragma GCC diagnostic pop
    }

    switch (page)
    {
        case Page::BIOS:
            cycles = biosMgr_.WriteMem(addr, val, length);
            break;
        case Page::EWRAM:
            cycles = WriteEWRAM(addr, val, length);
            break;
        case Page::IWRAM:
            cycles = WriteIWRAM(addr, val, length);
            break;
        case Page::IO:
            cycles = WriteIO(addr, val, length);
            break;
        case Page::PRAM:
            cycles = ppu_.WritePRAM(addr, val, length);
            break;
        case Page::VRAM:
            cycles = ppu_.WriteVRAM(addr, val, length);
            break;
        case Page::OAM:
            cycles = ppu_.WriteOAM(addr, val, length);
            break;
        case Page::GAMEPAK_MIN ... Page::GAMEPAK_MAX:
            cycles = gamePak_ ? gamePak_->WriteMem(addr, val, length) : 1;
            break;
        case Page::INVALID:
        default:
            cycles = 1;
            break;
    }

    return cycles;
}

MemReadData GameBoyAdvance::ReadEWRAM(u32 addr, AccessSize length)
{
    (void)addr;
    (void)length;
    return {1, 0, false};
}

int GameBoyAdvance::WriteEWRAM(u32 addr, u32 val, AccessSize length)
{
    (void)addr;
    (void)val;
    (void)length;
    return 1;
}

MemReadData GameBoyAdvance::ReadIWRAM(u32 addr, AccessSize length)
{
    (void)addr;
    (void)length;
    return {1, 0, false};
}

int GameBoyAdvance::WriteIWRAM(u32 addr, u32 val, AccessSize length)
{
    (void)addr;
    (void)val;
    (void)length;
    return 1;
}

MemReadData GameBoyAdvance::ReadIO(u32 addr, AccessSize length)
{
    (void)addr;
    (void)length;
    return {1, 0, false};
}

int GameBoyAdvance::WriteIO(u32 addr, u32 val, AccessSize length)
{
    (void)addr;
    (void)val;
    (void)length;
    return 1;
}
