#include <GBA/include/Debug/GameBoyAdvanceDebugger.hpp>
#include <GBA/include/BIOS/BIOSManager.hpp>
#include <GBA/include/Cartridge/GamePak.hpp>
#include <GBA/include/Debug/ArmDisassembler.hpp>
#include <GBA/include/Debug/PPUDebugger.hpp>
#include <GBA/include/Debug/ThumbDisassembler.hpp>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/PPU/PPU.hpp>
#include <GBA/include/Utilities/Types.hpp>

namespace debug
{
GameBoyAdvanceDebugger::GameBoyAdvanceDebugger(GameBoyAdvance const& gba) :
    gba_(gba),
    cpuDebugger_(gba.cpu_),
    ppuDebugger_(gba.ppu_)
{
}

DebugMemAccess GameBoyAdvanceDebugger::GetDebugMemAccess(u32 addr) const
{
    DebugMemAccess debugMem;
    auto page = GetMemPage(addr);
    debugMem.page = page;

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wswitch"
    switch (page)
    {
        case Page::BIOS:
        {
            debugMem.memoryBlock = gba_.biosMgr_.biosROM_;
            debugMem.minAddr = BIOS_ADDR_MIN;
            debugMem.AddrToIndex = [](u32 addr) {
                if (addr > BIOS_ADDR_MAX)
                {
                    return U32_MAX;
                }

                return addr;
            };

            break;
        }
        case Page::EWRAM:
        {
            debugMem.memoryBlock = gba_.EWRAM_;
            debugMem.minAddr = EWRAM_ADDR_MIN;
            debugMem.AddrToIndex = [](u32 addr) {
                if (addr > EWRAM_ADDR_MAX)
                {
                    addr = StandardMirroredAddress(addr, EWRAM_ADDR_MIN, EWRAM_ADDR_MAX);
                }

                return addr - EWRAM_ADDR_MIN;
            };

            break;
        }
        case Page::IWRAM:
        {
            debugMem.memoryBlock = gba_.IWRAM_;
            debugMem.minAddr = IWRAM_ADDR_MIN;
            debugMem.AddrToIndex = [](u32 addr) {
                if (addr > IWRAM_ADDR_MAX)
                {
                    addr = StandardMirroredAddress(addr, IWRAM_ADDR_MIN, IWRAM_ADDR_MAX);
                }

                return addr - IWRAM_ADDR_MIN;
            };

            break;
        }
        case Page::PRAM:
        {
            debugMem.memoryBlock = ppuDebugger_.GetPRAM();
            debugMem.minAddr = PRAM_ADDR_MIN;
            debugMem.AddrToIndex = [](u32 addr) {
                if (addr > PRAM_ADDR_MAX)
                {
                    addr = StandardMirroredAddress(addr, PRAM_ADDR_MIN, PRAM_ADDR_MAX);
                }

                return addr - PRAM_ADDR_MIN;
            };

            break;
        }
        case Page::VRAM:
        {
            debugMem.memoryBlock = ppuDebugger_.GetVRAM();
            debugMem.minAddr = VRAM_ADDR_MIN;
            debugMem.AddrToIndex = [](u32 addr) {
                if (addr > VRAM_ADDR_MAX)
                {
                    addr = StandardMirroredAddress(addr, VRAM_ADDR_MIN, VRAM_ADDR_MAX + (32 * KiB));

                    if (addr > VRAM_ADDR_MAX)
                    {
                        addr -= (32 * KiB);
                    }
                }

                return addr - VRAM_ADDR_MIN;
            };

            break;
        }
        case Page::OAM:
        {
            debugMem.memoryBlock = ppuDebugger_.GetOAM();
            debugMem.minAddr = OAM_ADDR_MIN;
            debugMem.AddrToIndex = [](u32 addr) {
                if (addr > OAM_ADDR_MAX)
                {
                    addr = StandardMirroredAddress(addr, OAM_ADDR_MIN, OAM_ADDR_MAX);
                }

                return addr - OAM_ADDR_MIN;
            };

            break;
        }
        case Page{0x08} ... Page{0x09}:
        {
            debugMem.memoryBlock = gba_.gamePak_ ? gba_.gamePak_->ROM_ : std::span<const std::byte>{};
            debugMem.minAddr = GAMEPAK_ROM_ADDR_MIN;
            debugMem.AddrToIndex = [](u32 addr) {
                return addr - GAMEPAK_ROM_ADDR_MIN;
            };

            break;
        }
        case Page{0x0A} ... Page{0x0B}:
        {
            debugMem.memoryBlock = gba_.gamePak_ ? gba_.gamePak_->ROM_ : std::span<const std::byte>{};
            debugMem.minAddr = GAMEPAK_ROM_ADDR_MIN + (32 * MiB);
            debugMem.AddrToIndex = [](u32 addr) {
                return addr - GAMEPAK_ROM_ADDR_MIN - (32 * MiB);
            };

            break;
        }
        case Page{0x0C} ... Page{0x0D}:
        {
            debugMem.memoryBlock = gba_.gamePak_ ? gba_.gamePak_->ROM_ : std::span<const std::byte>{};
            debugMem.minAddr = GAMEPAK_ROM_ADDR_MIN + (64 * MiB);
            debugMem.AddrToIndex = [](u32 addr) {
                return addr - GAMEPAK_ROM_ADDR_MIN - (2 * (32 * MiB));
            };

            break;
        }
        case Page::INVALID:
        default:
        {
            debugMem.memoryBlock = std::span<const std::byte>{};
            debugMem.minAddr = 0;
            debugMem.page = Page::INVALID;
            debugMem.AddrToIndex = [](u32 addr) {
                (void)addr;
                return U32_MAX;
            };

            break;
        }
    }
    #pragma GCC diagnostic pop

    return debugMem;
}

u32 GameBoyAdvanceDebugger::ReadRegister(u32 addr, AccessSize length) const
{
    switch (addr)
    {
        case LCD_IO_ADDR_MIN ... LCD_IO_ADDR_MAX:
            return ppuDebugger_.ReadRegister(addr, length);
        case SOUND_IO_ADDR_MIN ... SOUND_IO_ADDR_MAX:
            break;
        case DMA_IO_ADDR_MIN ... DMA_IO_ADDR_MAX:
            break;
        case TIMER_IO_ADDR_MIN ... TIMER_IO_ADDR_MAX:
            break;
        case SERIAL_IO_1_ADDR_MIN ... SERIAL_IO_1_ADDR_MAX:
            break;
        case KEYPAD_IO_ADDR_MIN ... KEYPAD_IO_ADDR_MAX:
            break;
        case SERIAL_IO_2_ADDR_MIN ... SERIAL_IO_2_ADDR_MAX:
            break;
        case SYSTEM_CONTROL_IO_ADDR_MIN ... SYSTEM_CONTROL_IO_ADDR_MAX:
            break;
        default:
            break;
    }

    return 0;
}

CpuDebugInfo GameBoyAdvanceDebugger::GetCpuDebugInfo() const
{
    CpuDebugInfo debugInfo;

    // const_cast because GetPC was made non-const to avoid issues with Functor not accepting a const member function.
    debugInfo.pcMem = GetDebugMemAccess(const_cast<cpu::ARM7TDMI*>(&gba_.cpu_)->GetPC());

    debugInfo.spMem = GetDebugMemAccess(gba_.cpu_.GetSP());
    cpuDebugger_.PopulateCpuRegState(debugInfo.regState);
    debugInfo.nextAddrToExecute = gba_.cpu_.GetNextAddrToExecute();
    return debugInfo;
}
}  // debug
