#include <GBA/include/PPU/PPU.hpp>
#include <array>
#include <cstddef>
#include <cstring>
#include <functional>
#include <span>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/PPU/Registers.hpp>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Types.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>

namespace graphics
{
PPU::PPU(EventScheduler& scheduler, SystemControl& systemControl) : scheduler_(scheduler), systemControl_(systemControl)
{
    bg2RefX_ = 0;
    bg2RefY_ = 0;
    bg3RefX_ = 0;
    bg3RefY_ = 0;

    PRAM_.fill(std::byte{0});
    OAM_.fill(std::byte{0});
    VRAM_.fill(std::byte{0});
    registers_.fill(std::byte{0});

    scheduler_.RegisterEvent(EventType::VDraw, std::bind(&PPU::HBlank, this, std::placeholders::_1));
    scheduler_.ScheduleEvent(EventType::HBlank, 960 + 46);
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Bus functionality
///---------------------------------------------------------------------------------------------------------------------------------

MemReadData PPU::ReadPRAM(u32 addr, AccessSize length)
{
    if (addr > PRAM_ADDR_MAX)
    {
        addr = StandardMirroredAddress(addr, PRAM_ADDR_MIN, PRAM_ADDR_MAX);
    }

    u32 val = ReadMemoryBlock(PRAM_, addr, PRAM_ADDR_MIN, length);
    int cycles = (length == AccessSize::WORD) ? 2 : 1;
    return {cycles, val, false};
}

int PPU::WritePRAM(u32 addr, u32 val, AccessSize length)
{
    if (length == AccessSize::BYTE)
    {
        length = AccessSize::HALFWORD;
        addr &= ~0x01;
        val = (val & U8_MAX) * 0x0101;
    }

    if (addr > PRAM_ADDR_MAX)
    {
        addr = StandardMirroredAddress(addr, PRAM_ADDR_MIN, PRAM_ADDR_MAX);
    }

    WriteMemoryBlock(PRAM_, addr, PRAM_ADDR_MIN, val, length);
    return (length == AccessSize::WORD) ? 2 : 1;
}

MemReadData PPU::ReadOAM(u32 addr, AccessSize length)
{
    if (addr > OAM_ADDR_MAX)
    {
        addr = StandardMirroredAddress(addr, OAM_ADDR_MIN, OAM_ADDR_MAX);
    }

    u32 val = ReadMemoryBlock(OAM_, addr, OAM_ADDR_MIN, length);
    return {1, val, false};
}

int PPU::WriteOAM(u32 addr, u32 val, AccessSize length)
{
    if (length == AccessSize::BYTE)
    {
        return 1;
    }

    if (addr > OAM_ADDR_MAX)
    {
        addr = StandardMirroredAddress(addr, OAM_ADDR_MIN, OAM_ADDR_MAX);
    }

    WriteMemoryBlock(OAM_, addr, OAM_ADDR_MIN, val, length);
    return 1;
}

MemReadData PPU::ReadVRAM(u32 addr, AccessSize length)
{
    if (addr > VRAM_ADDR_MAX)
    {
        addr = StandardMirroredAddress(addr, VRAM_ADDR_MIN, VRAM_ADDR_MAX + (32 * KiB));

        if (addr > VRAM_ADDR_MAX)
        {
            addr -= (32 * KiB);
        }
    }

    u32 val = ReadMemoryBlock(VRAM_, addr, VRAM_ADDR_MIN, length);
    int cycles = (length == AccessSize::WORD) ? 2 : 1;
    return {cycles, val, false};
}

int PPU::WriteVRAM(u32 addr, u32 val, AccessSize length)
{
    if (addr > VRAM_ADDR_MAX)
    {
        addr = StandardMirroredAddress(addr, VRAM_ADDR_MIN, VRAM_ADDR_MAX + (32 * KiB));

        if (addr > VRAM_ADDR_MAX)
        {
            addr -= (32 * KiB);
        }
    }

    if (length == AccessSize::BYTE)
    {
        DISPCNT dispcnt = GetDISPCNT();

        if ((addr > 0x0601'4000) || ((addr > 0x0601'0000) && (dispcnt.bgMode < 3)))
        {
            // Ignore byte writes to OBJ tiles.
            return 1;
        }

        length = AccessSize::HALFWORD;
        addr &= ~0x01;
        val = (val & U8_MAX) * 0x0101;
    }

    WriteMemoryBlock(VRAM_, addr, VRAM_ADDR_MIN, val, length);
    return (length == AccessSize::WORD) ? 2 : 1;
}

MemReadData PPU::ReadReg(u32 addr, AccessSize length)
{
    if (((0x0400'0010 <= addr) && (addr < 0x0400'0048)) ||
        ((0x0400'004C <= addr) && (addr < 0x0400'0050)) ||
        (addr >= 0x0400'0054))
    {
        // Attempting to read unused or write-only registers.
        return {1, 0, true};
    }

    u32 val = ReadMemoryBlock(registers_, addr, LCD_IO_ADDR_MIN, length);
    return {1, val, false};
}

int PPU::WriteReg(u32 addr, u32 val, AccessSize length)
{
    if ((0x0400'0004 <= addr) && (addr < 0x0400'0008))
    {
        WriteDispstatVcount(addr, val, length);
        return 1;
    }

    WriteMemoryBlock(registers_, addr, LCD_IO_ADDR_MIN, val, length);

    if ((0x0400'0028 <= addr) && (addr < 0x0400'002C))
    {
        SetBG2RefX();
    }
    else if ((0x0400'002C <= addr) && (addr < 0x0400'0030))
    {
        SetBG2RefY();
    }
    else if ((0x0400'0038 <= addr) && (addr < 0x0400'003C))
    {
        SetBG3RefX();
    }
    else if ((0x0400'003C <= addr) && (addr < 0x0400'0040))
    {
        SetBG3RefY();
    }

    return 1;
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Register access/updates
///---------------------------------------------------------------------------------------------------------------------------------

void PPU::SetBG2RefX()
{
    std::memcpy(&bg2RefX_, &registers_[0x28], sizeof(bg2RefX_));
    bg2RefX_ = SignExtend<i32, 27>(bg2RefX_);
}

void PPU::SetBG2RefY()
{
    std::memcpy(&bg2RefY_, &registers_[0x28], sizeof(bg2RefY_));
    bg2RefY_ = SignExtend<i32, 27>(bg2RefY_);
}

void PPU::SetBG3RefX()
{
    std::memcpy(&bg3RefX_, &registers_[0x28], sizeof(bg3RefX_));
    bg3RefX_ = SignExtend<i32, 27>(bg3RefX_);
}

void PPU::SetBG3RefY()
{
    std::memcpy(&bg3RefY_, &registers_[0x28], sizeof(bg3RefY_));
    bg3RefY_ = SignExtend<i32, 27>(bg3RefY_);
}

void PPU::WriteDispstatVcount(u32 addr, u32 val, AccessSize length)
{
    // Ignore BYTE or HALFWORD writes to VCOUNT
    if (addr < 0x0400'0006)
    {
        DISPSTAT prevDispStat = GetDISPSTAT();
        DISPSTAT newDispStat;
        std::memcpy(&newDispStat, &val, sizeof(DISPSTAT));

        if (length != AccessSize::BYTE)
        {
            // Write to both bytes of DISPSTAT, restore all read only fields
            newDispStat.vBlank = prevDispStat.vBlank;
            newDispStat.hBlank = prevDispStat.hBlank;
            newDispStat.vCounter = prevDispStat.vCounter;
            newDispStat.unusedReadOnly = prevDispStat.unusedReadOnly;
        }
        else if (addr == 0x0400'0004)
        {
            // Write to lower byte of DISPSTAT, restore all read only fields and upper byte
            newDispStat.vBlank = prevDispStat.vBlank;
            newDispStat.hBlank = prevDispStat.hBlank;
            newDispStat.vCounter = prevDispStat.vCounter;
            newDispStat.unusedReadOnly = prevDispStat.unusedReadOnly;
            newDispStat.vCountSetting = prevDispStat.vCountSetting;
        }
        else
        {
            // Write to upper byte of DISPSTAT, restore all fields in the lower byte
            newDispStat.vBlank = prevDispStat.vBlank;
            newDispStat.hBlank = prevDispStat.hBlank;
            newDispStat.vCounter = prevDispStat.vCounter;
            newDispStat.vBlankIrqEnable = prevDispStat.vBlankIrqEnable;
            newDispStat.hBlankIrqEnable = prevDispStat.hBlankIrqEnable;
            newDispStat.vCounterIrqEnable = prevDispStat.vCounterIrqEnable;
            newDispStat.unusedReadOnly = prevDispStat.unusedReadOnly;
            newDispStat.unusedReadWrite = prevDispStat.unusedReadWrite;
        }

        SetDISPSTAT(newDispStat);

        if (prevDispStat.vCountSetting != newDispStat.vCountSetting)
        {
            CheckVCountSetting();
        }
    }
}

void PPU::CheckVCountSetting()
{
    u8 currentScanline = GetVCOUNT();
    auto dispstat = GetDISPSTAT();

    if (currentScanline == dispstat.vCountSetting)
    {
        dispstat.vCounter = 1;

        if (dispstat.vCounterIrqEnable)
        {
            // TODO: Request VCOUNT IRQ
        }
    }
    else
    {
        dispstat.vCounter = 0;
    }

    SetDISPSTAT(dispstat);
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Event Handlers
///---------------------------------------------------------------------------------------------------------------------------------

void PPU::HBlank(int extraCycles)
{
    // Register updates
    auto dispstat = GetDISPSTAT();
    dispstat.hBlank = 1;

    if (dispstat.hBlankIrqEnable)
    {
        // TODO: Request HBlank IRQ
    }

    SetDISPSTAT(dispstat);

    // Schedule next PPU event
    u8 scanline = GetVCOUNT();
    int cyclesUntilEvent = (272 - 46) - extraCycles;
    EventType event = ((scanline < 159) || (scanline == 227)) ? EventType::VDraw : EventType::VBlank;
    scheduler_.ScheduleEvent(event, cyclesUntilEvent);

    // Render the current scanline
    if (scanline < 160)
    {
        EvaluateScanline();
    }
}

void PPU::VBlank(int extraCycles)
{
    // Register updates
    auto dispstat = GetDISPSTAT();
    u8 scanline = GetVCOUNT() + 1;

    dispstat.hBlank = 0;

    if (scanline == 160)
    {
        dispstat.vBlank = 1;
        frameBuffer_.ResetFrameIndex();

        if (dispstat.vBlankIrqEnable)
        {
            // TODO: Request VBlank IRQ
        }

        SetBG2RefX();
        SetBG2RefY();
        SetBG3RefX();
        SetBG3RefY();
    }
    else if (scanline == 227)
    {
        dispstat.vBlank = 0;
    }

    SetDISPSTAT(dispstat);
    SetVCOUNT(scanline);
    CheckVCountSetting();

    // Schedule next PPU event
    int cyclesUntilEvent = (960 + 46) - extraCycles;
    scheduler_.ScheduleEvent(EventType::HBlank, cyclesUntilEvent);
}

void PPU::VDraw(int extraCycles)
{
    // Register updates
    auto dispstat = GetDISPSTAT();
    u8 scanline = GetVCOUNT() + 1;

    if (scanline == 228)
    {
        scanline = 0;
    }

    dispstat.vBlank = 0;
    dispstat.hBlank = 0;

    SetDISPSTAT(dispstat);
    SetVCOUNT(scanline);
    CheckVCountSetting();

    // Schedule next PPU event
    int cyclesUntilEvent = (960 + 46) - extraCycles;
    scheduler_.ScheduleEvent(EventType::HBlank, cyclesUntilEvent);
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Rendering
///---------------------------------------------------------------------------------------------------------------------------------

void PPU::EvaluateScanline()
{
    auto dispcnt = GetDISPCNT();
    u16 backdropColor = GetBgColor(0);

    switch (dispcnt.bgMode)
    {
        case 3:
            RenderMode3Scanline();
            break;
        case 4:
            RenderMode4Scanline();
            break;
        default:
            break;
    }

    frameBuffer_.RenderScanline(backdropColor, dispcnt.forceBlank);
}

void PPU::RenderMode3Scanline()
{
    auto dispcnt = GetDISPCNT();
    auto bg2cnt = GetBGCNT(2);
    u8 scanline = GetVCOUNT();

    if (!dispcnt.screenDisplayBg2)
    {
        return;
    }

    std::span<u16> bitmap(reinterpret_cast<u16*>(VRAM_.data()), LCD_WIDTH * LCD_HEIGHT);
    size_t bitmapIndex = scanline * LCD_WIDTH;

    for (u8 dot = 0; dot < LCD_WIDTH; ++dot)
    {
        frameBuffer_.PushPixel({PixelSrc::BG2, bitmap[bitmapIndex++], bg2cnt.priority, false}, dot);
    }
}

void PPU::RenderMode4Scanline()
{
    auto dispcnt = GetDISPCNT();
    auto bg2cnt = GetBGCNT(2);
    u8 scanline = GetVCOUNT();

    if (!dispcnt.screenDisplayBg2)
    {
        return;
    }

    size_t bitmapIndex = scanline * LCD_WIDTH;

    if (dispcnt.displayFrameSelect)
    {
        bitmapIndex += 0xA000;
    }

    for (u8 dot = 0; dot < LCD_WIDTH; ++dot)
    {
        u8 paletteIndex = static_cast<u8>(VRAM_[bitmapIndex++]);
        u16 color = GetBgColor(paletteIndex);
        bool transparent = (paletteIndex == 0);
        frameBuffer_.PushPixel({PixelSrc::BG2, color, bg2cnt.priority, transparent}, dot);
    }
}
}  // namespace graphics
