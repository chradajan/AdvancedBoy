#include <GBA/include/GameBoyAdvance.hpp>
#include <array>
#include <cstddef>
#include <filesystem>
#include <functional>
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

GameBoyAdvance::GameBoyAdvance(fs::path biosPath, fs::path romPath, fs::path logDir) :
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
    lastSuccessfulFetch_(0)
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
            MainLoop(samplesToGenerate);
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

void GameBoyAdvance::MainLoop(size_t samples)
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
            cpu_.Step(systemControl_.IrqPending());
        }
    }
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Bus functionality
///---------------------------------------------------------------------------------------------------------------------------------

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
