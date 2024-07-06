#include <GBA/include/GameBoyAdvance.hpp>
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

namespace
{
Address ForceAlignAddress(Address addr, AccessSize length)
{
    u32 alignment = static_cast<u32>(length) - 1;

    if ((addr & alignment) != 0)
    {
        addr &= ~alignment;
    }

    return addr;
}
}

std::pair<u32, CpuCycles> GameBoyAdvance::ReadMem(Address addr, AccessSize length)
{
    addr = ForceAlignAddress(addr, length);
    auto page = Page::INVALID;

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
            break;
        case Page::EWRAM:
            break;
        case Page::IWRAM:
            break;
        case Page::IO:
            break;
        case Page::PRAM:
            break;
        case Page::VRAM:
            break;
        case Page::OAM:
            break;
        case Page::GAMEPAK_MIN:
            break;
        case Page::GAMEPAK_MAX:
            break;
        case Page::INVALID:
            break;
    }

    return {0, ONE_CYCLE};
}

CpuCycles GameBoyAdvance::WriteMem(Address addr, u32 val, AccessSize length)
{
    (void)val;

    addr = ForceAlignAddress(addr, length);
    auto page = Page::INVALID;

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
            break;
        case Page::EWRAM:
            break;
        case Page::IWRAM:
            break;
        case Page::IO:
            break;
        case Page::PRAM:
            break;
        case Page::VRAM:
            break;
        case Page::OAM:
            break;
        case Page::GAMEPAK_MIN:
            break;
        case Page::GAMEPAK_MAX:
            break;
        case Page::INVALID:
            break;
    }

    return ONE_CYCLE;
}
