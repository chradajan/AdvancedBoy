#include <GBA/include/GameBoyAdvance.hpp>
#include <array>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <memory>
#include <span>
#include <unordered_map>
#include <GBA/include/APU/APU.hpp>
#include <GBA/include/BIOS/BIOSManager.hpp>
#include <GBA/include/Cartridge/GamePak.hpp>
#include <GBA/include/CPU/ARM7TDMI.hpp>
#include <GBA/include/DMA/DmaManager.hpp>
#include <GBA/include/Keypad/Keypad.hpp>
#include <GBA/include/Keypad/Registers.hpp>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/PPU/PPU.hpp>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Timers/TimerManager.hpp>
#include <GBA/include/Types/Types.hpp>
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

GameBoyAdvance::GameBoyAdvance(fs::path biosPath,
                               fs::path romPath,
                               fs::path logDir,
                               std::function<void(int)> vBlankCallback,
                               std::function<void()> breakpointCallback) :
    scheduler_(),
    log_(logDir, scheduler_),
    systemControl_(scheduler_, log_),
    apu_(scheduler_),
    biosMgr_(biosPath, {&cpu::ARM7TDMI::GetPC, cpu_}),
    cpu_({&GameBoyAdvance::ReadMem, *this}, {&GameBoyAdvance::WriteMem, *this}, scheduler_, log_),
    dmaMgr_({&GameBoyAdvance::ReadMem, *this}, {&GameBoyAdvance::WriteMem, *this}, scheduler_, systemControl_),
    keypad_(systemControl_),
    ppu_(scheduler_, systemControl_),
    timerMgr_(scheduler_, systemControl_),
    gamePak_(nullptr),
    lastSuccessfulFetch_(0),
    BreakpointCallback(breakpointCallback),
    breakpointCycle_(U64_MAX)
{
    if (!romPath.empty() && fs::exists(romPath))
    {
        gamePak_ = std::make_unique<cartridge::GamePak>(romPath, scheduler_, systemControl_);

        if (!gamePak_->GamePakLoaded())
        {
            gamePak_.reset();
        }
    }

    dmaMgr_.ConnectGamePak(gamePak_.get());
    EWRAM_.fill(std::byte{0});
    IWRAM_.fill(std::byte{0});

    scheduler_.RegisterEvent(EventType::HBlank, std::bind(&GameBoyAdvance::HBlank, this, std::placeholders::_1));
    scheduler_.RegisterEvent(EventType::VBlank, std::bind(&GameBoyAdvance::VBlank, this, std::placeholders::_1));
    scheduler_.RegisterEvent(EventType::Timer0Overflow, std::bind(&GameBoyAdvance::Timer0Overflow, this, std::placeholders::_1));
    scheduler_.RegisterEvent(EventType::Timer1Overflow, std::bind(&GameBoyAdvance::Timer1Overflow, this, std::placeholders::_1));
    scheduler_.RegisterEvent(EventType::Timer2Overflow, std::bind(&GameBoyAdvance::Timer2Overflow, this, std::placeholders::_1));
    scheduler_.RegisterEvent(EventType::Timer3Overflow, std::bind(&GameBoyAdvance::Timer3Overflow, this, std::placeholders::_1));
    scheduler_.RegisterEvent(EventType::NotifyVBlank, vBlankCallback);

    if (log_.Enabled())
    {
        RunDisassembler();
    }
}

GameBoyAdvance::~GameBoyAdvance()
{
    if (gamePak_)
    {
        gamePak_->Save();
    }

    log_.DumpRemainingBuffer();
}

void GameBoyAdvance::Run()
{
    size_t samplesToGenerate = apu_.FreeBufferSpace();

    while (samplesToGenerate > 0)
    {
        try
        {
            bool hitBreakpoint = MainLoop(samplesToGenerate);

            if (hitBreakpoint)
            {
                BreakpointCallback();
                return;
            }
        }
        catch (std::exception& e)
        {
            log_.LogException(e);
            log_.DumpRemainingBuffer();
            throw;
        }

        samplesToGenerate = apu_.FreeBufferSpace();
    }
}

bool GameBoyAdvance::MainLoop(size_t samples)
{
    apu_.ClearSampleCounter();

    while (apu_.GetSampleCounter() < samples)
    {
        if (dmaMgr_.DmaRunning() || systemControl_.Halted())
        {
            scheduler_.FireNextEvent();
        }
        else
        {
            if (breakpoints_.contains(cpu_.GetNextAddrToExecute()) && (breakpointCycle_ != scheduler_.GetTotalElapsedCycles()))
            {
                breakpointCycle_ = scheduler_.GetTotalElapsedCycles();
                return true;
            }

            cpu_.Step(systemControl_.IrqPending());
        }
    }

    return false;
}

void GameBoyAdvance::SingleStep()
{
    while (true)
    {
        while (dmaMgr_.DmaRunning() || systemControl_.Halted())
        {
            scheduler_.FireNextEvent();
        }

        if (cpu_.Step(systemControl_.IrqPending()))
        {
            break;
        }
    }
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Bus functionality
///---------------------------------------------------------------------------------------------------------------------------------

std::pair<u32, int> GameBoyAdvance::ReadMem(u32 addr, AccessSize length)
{
    addr = ForceAlignAddress(addr, length);
    MemReadData readData;
    auto page = GetMemPage(addr);

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
        readData.Value = lastSuccessfulFetch_;
        readData.Cycles = 1;
    }
    else
    {
        lastSuccessfulFetch_ = readData.Value;
    }

    return {readData.Value, readData.Cycles};
}

int GameBoyAdvance::WriteMem(u32 addr, u32 val, AccessSize length)
{
    addr = ForceAlignAddress(addr, length);
    int cycles = 1;
    auto page = GetMemPage(addr);

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
    if (addr > EWRAM_ADDR_MAX)
    {
        addr = StandardMirroredAddress(addr, EWRAM_ADDR_MIN, EWRAM_ADDR_MAX);
    }

    u32 val = ReadMemoryBlock(EWRAM_, addr, EWRAM_ADDR_MIN, length);
    int cycles = (length == AccessSize::WORD) ? 6 : 3;
    return {cycles, val, false};
}

int GameBoyAdvance::WriteEWRAM(u32 addr, u32 val, AccessSize length)
{
    if (addr > EWRAM_ADDR_MAX)
    {
        addr = StandardMirroredAddress(addr, EWRAM_ADDR_MIN, EWRAM_ADDR_MAX);
    }

    WriteMemoryBlock(EWRAM_, addr, EWRAM_ADDR_MIN, val, length);
    return (length == AccessSize::WORD) ? 6 : 3;
}

MemReadData GameBoyAdvance::ReadIWRAM(u32 addr, AccessSize length)
{
    if (addr > IWRAM_ADDR_MAX)
    {
        addr = StandardMirroredAddress(addr, IWRAM_ADDR_MIN, IWRAM_ADDR_MAX);
    }

    u32 val = ReadMemoryBlock(IWRAM_, addr, IWRAM_ADDR_MIN, length);
    return {1, val, false};
}

int GameBoyAdvance::WriteIWRAM(u32 addr, u32 val, AccessSize length)
{
    if (addr > IWRAM_ADDR_MAX)
    {
        addr = StandardMirroredAddress(addr, IWRAM_ADDR_MIN, IWRAM_ADDR_MAX);
    }

    WriteMemoryBlock(IWRAM_, addr, IWRAM_ADDR_MIN, val, length);
    return 1;
}

MemReadData GameBoyAdvance::ReadIO(u32 addr, AccessSize length)
{
    if ((addr > SYSTEM_CONTROL_IO_ADDR_MAX) && (((addr - 0x0400'0800) % (64 * KiB)) < 4))
    {
        // I/O registers are not mirrored, with the exception of 4000800h repeating every 64K.
        addr = 0x0400'0800 + ((addr - 0x0400'0800) % (64 * KiB));
    }

    MemReadData readData;

    switch (addr)
    {
        case LCD_IO_ADDR_MIN ... LCD_IO_ADDR_MAX:
            readData = ppu_.ReadReg(addr, length);
            break;
        case SOUND_IO_ADDR_MIN ... SOUND_IO_ADDR_MAX:
            readData = apu_.ReadReg(addr, length);
            break;
        case DMA_IO_ADDR_MIN ... DMA_IO_ADDR_MAX:
            readData = dmaMgr_.ReadReg(addr, length);
            break;
        case TIMER_IO_ADDR_MIN ... TIMER_IO_ADDR_MAX:
            readData = timerMgr_.ReadReg(addr, length);
            break;
        case SERIAL_IO_1_ADDR_MIN ... SERIAL_IO_1_ADDR_MAX:
            readData = {1, 0, false};
            break;
        case KEYPAD_IO_ADDR_MIN ... KEYPAD_IO_ADDR_MAX:
            readData = keypad_.ReadReg(addr, length);
            break;
        case SERIAL_IO_2_ADDR_MIN ... SERIAL_IO_2_ADDR_MAX:
            readData = {1, 0, false};
            break;
        case SYSTEM_CONTROL_IO_ADDR_MIN ... SYSTEM_CONTROL_IO_ADDR_MAX:
            readData = systemControl_.ReadReg(addr, length);
            break;
        default:
            readData = {1, 0, true};
            break;
    }

    return readData;
}

int GameBoyAdvance::WriteIO(u32 addr, u32 val, AccessSize length)
{
    if ((addr > SYSTEM_CONTROL_IO_ADDR_MAX) && (((addr - 0x0400'0800) % (64 * KiB)) < 4))
    {
        // I/O registers are not mirrored, with the exception of 4000800h repeating every 64K.
        addr = 0x0400'0800 + ((addr - 0x0400'0800) % (64 * KiB));
    }

    int cycles;

    switch (addr)
    {
        case LCD_IO_ADDR_MIN ... LCD_IO_ADDR_MAX:
            cycles = ppu_.WriteReg(addr, val, length);
            break;
        case SOUND_IO_ADDR_MIN ... SOUND_IO_ADDR_MAX:
            cycles = apu_.WriteReg(addr, val, length);
            break;
        case DMA_IO_ADDR_MIN ... DMA_IO_ADDR_MAX:
            cycles = dmaMgr_.WriteReg(addr, val, length);
            break;
        case TIMER_IO_ADDR_MIN ... TIMER_IO_ADDR_MAX:
            cycles = timerMgr_.WriteReg(addr, val, length);
            break;
        case SERIAL_IO_1_ADDR_MIN ... SERIAL_IO_1_ADDR_MAX:
            cycles = 1;
            break;
        case KEYPAD_IO_ADDR_MIN ... KEYPAD_IO_ADDR_MAX:
            cycles = keypad_.WriteReg(addr, val, length);
            break;
        case SERIAL_IO_2_ADDR_MIN ... SERIAL_IO_2_ADDR_MAX:
            cycles = 1;
            break;
        case SYSTEM_CONTROL_IO_ADDR_MIN ... SYSTEM_CONTROL_IO_ADDR_MAX:
            cycles = systemControl_.WriteReg(addr, val, length);
            break;
        default:
            cycles = 1;
            break;
    }

    return cycles;
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Event handling
///---------------------------------------------------------------------------------------------------------------------------------

void GameBoyAdvance::HBlank(int extraCycles)
{
    ppu_.HBlank(extraCycles);

    if (ppu_.GetVCOUNT() < 160)
    {
        dmaMgr_.CheckHBlank();
    }
}

void GameBoyAdvance::VBlank(int extraCycles)
{
    ppu_.VBlank(extraCycles);

    if (ppu_.GetVCOUNT() == 160)
    {
        dmaMgr_.CheckVBlank();
    }
}

void GameBoyAdvance::TimerOverflow(u8 index, int extraCycles)
{
    timerMgr_.TimerOverflow(index, extraCycles);
    auto [replenishA, replenishB] = apu_.TimerOverflow(index);

    if (replenishA)
    {
        dmaMgr_.CheckFifoA();
    }

    if (replenishB)
    {
        dmaMgr_.CheckFifoB();
    }
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Debug
///---------------------------------------------------------------------------------------------------------------------------------

debug::cpu::CpuDebugInfo GameBoyAdvance::GetCpuDebugInfo()
{
    debug::cpu::CpuDebugInfo debugInfo;
    debugInfo.pcMem = GetDebugMemAccess(cpu_.GetPC());
    debugInfo.spMem = GetDebugMemAccess(cpu_.GetSP());
    cpu_.GetRegState(debugInfo.regState);
    debugInfo.nextAddrToExecute = cpu_.GetNextAddrToExecute();
    return debugInfo;
}

void GameBoyAdvance::RunDisassembler()
{
    if (biosMgr_.BiosLoaded())
    {
        auto bios = biosMgr_.GetBIOS();

        for (u32 i = 0; (i + 4) <= bios.size(); i += 4)
        {
            u32 instruction = MemCpyInit<u32>(&bios[i]);
            cpu_.DisassembleArmInstruction(instruction);
        }

        for (u32 i = 0; (i + 2) <= bios.size(); i += 2)
        {
            u16 instruction = MemCpyInit<u16>(&bios[i]);
            cpu_.DisassembleThumbInstruction(instruction);
        }
    }

    if (gamePak_)
    {
        auto rom = gamePak_->GetROM();

        for (u32 index = 0; (index + 4) <= rom.size(); index += 4)
        {
            u32 instruction = MemCpyInit<u32>(&rom[index]);
            cpu_.DisassembleArmInstruction(instruction);
        }

        for (u32 index = 0; (index + 2) <= rom.size(); index += 2)
        {
            u16 instruction = MemCpyInit<u16>(&rom[index]);
            cpu_.DisassembleThumbInstruction(instruction);
        }
    }
}

debug::DebugMemAccess GameBoyAdvance::GetDebugMemAccess(u32 addr)
{
    debug::DebugMemAccess debugMem;
    auto page = GetMemPage(addr);
    debugMem.page = page;

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wswitch"
    switch (page)
    {
        case Page::BIOS:
        {
            debugMem.memoryBlock = biosMgr_.GetBIOS();
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
            debugMem.memoryBlock = EWRAM_;
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
            debugMem.memoryBlock = IWRAM_;
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
            debugMem.memoryBlock = ppu_.GetPRAM();
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
            debugMem.memoryBlock = ppu_.GetVRAM();
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
            debugMem.memoryBlock = ppu_.GetOAM();
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
            debugMem.memoryBlock = gamePak_ ? gamePak_->GetROM() : std::span<const std::byte>{};
            debugMem.minAddr = GAMEPAK_ROM_ADDR_MIN;
            debugMem.AddrToIndex = [](u32 addr) {
                return addr - GAMEPAK_ROM_ADDR_MIN;
            };

            break;
        }
        case Page{0x0A} ... Page{0x0B}:
        {
            debugMem.memoryBlock = gamePak_ ? gamePak_->GetROM() : std::span<const std::byte>{};
            debugMem.minAddr = GAMEPAK_ROM_ADDR_MIN + (32 * MiB);
            debugMem.AddrToIndex = [](u32 addr) {
                return addr - GAMEPAK_ROM_ADDR_MIN - (32 * MiB);
            };

            break;
        }
        case Page{0x0C} ... Page{0x0D}:
        {
            debugMem.memoryBlock = gamePak_ ? gamePak_->GetROM() : std::span<const std::byte>{};
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
