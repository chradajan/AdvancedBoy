#include <GBA/include/PPU/PPU.hpp>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <functional>
#include <span>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/PPU/Registers.hpp>
#include <GBA/include/PPU/VramViews.hpp>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>
#include <GBA/include/Utilities/Types.hpp>

namespace graphics
{
PPU::PPU(EventScheduler& scheduler, SystemControl& systemControl) : scheduler_(scheduler), systemControl_(systemControl)
{
    window0EnabledOnScanline_ = false;
    window1EnabledOnScanline_ = false;

    bg2RefX_ = 0;
    bg2RefY_ = 0;
    bg3RefX_ = 0;
    bg3RefY_ = 0;

    fpsCounter_ = 0;

    PRAM_.fill(std::byte{0});
    OAM_.fill(std::byte{0});
    VRAM_.fill(std::byte{0});
    registers_.fill(std::byte{0});

    scheduler_.RegisterEvent(EventType::VDraw, std::bind(&PPU::VDraw, this, std::placeholders::_1));
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
    std::memcpy(&bg2RefX_, &registers_[0x28], sizeof(i32));
    bg2RefX_ = SignExtend<i32, 27>(bg2RefX_);
}

void PPU::SetBG2RefY()
{
    std::memcpy(&bg2RefY_, &registers_[0x2C], sizeof(i32));
    bg2RefY_ = SignExtend<i32, 27>(bg2RefY_);
}

void PPU::SetBG3RefX()
{
    std::memcpy(&bg3RefX_, &registers_[0x38], sizeof(i32));
    bg3RefX_ = SignExtend<i32, 27>(bg3RefX_);
}

void PPU::SetBG3RefY()
{
    std::memcpy(&bg3RefY_, &registers_[0x3C], sizeof(i32));
    bg3RefY_ = SignExtend<i32, 27>(bg3RefY_);
}

void PPU::IncrementAffineBackgroundReferencePoints()
{
    bg2RefX_ += MemCpyInit<i16>(&registers_[0x22]);
    bg2RefY_ += MemCpyInit<i16>(&registers_[0x26]);
    bg3RefX_ += MemCpyInit<i16>(&registers_[0x32]);
    bg3RefY_ += MemCpyInit<i16>(&registers_[0x36]);
}

void PPU::WriteDispstatVcount(u32 addr, u32 val, AccessSize length)
{
    // Ignore BYTE or HALFWORD writes to VCOUNT
    if (addr < 0x0400'0006)
    {
        u16 writableMask;

        if (length != AccessSize::BYTE)
        {
            // Write both bytes of DISPSTAT
            writableMask = 0xFFB8;
            val = (val & U16_MAX) & writableMask;
        }
        else if (addr == 0x0400'0004)
        {
            // Write to lower byte of DISPSTAT
            writableMask = 0x00B8;
            val = (val & U8_MAX) & writableMask;
        }
        else
        {
            // Write to upper byte of DISPSTAT
            writableMask = 0xFF00;
            val = ((val & U8_MAX) << 8) & writableMask;
        }

        auto prevDispStat = GetDISPSTAT();
        u16 prevDispStatValue = std::bit_cast<u16, DISPSTAT>(prevDispStat);
        prevDispStatValue &= ~writableMask;

        u16 newDispStatValue = prevDispStatValue | val;
        auto newDispStat = std::bit_cast<DISPSTAT, u16>(newDispStatValue);

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
            systemControl_.RequestInterrupt(InterruptType::LCD_VCOUNTER_MATCH);
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
        systemControl_.RequestInterrupt(InterruptType::LCD_HBLANK);
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
        ++fpsCounter_;

        if (dispstat.vBlankIrqEnable)
        {
            systemControl_.RequestInterrupt(InterruptType::LCD_VBLANK);
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
    SetNonObjWindowEnabled();

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
    SetNonObjWindowEnabled();

    // Schedule next PPU event
    int cyclesUntilEvent = (960 + 46) - extraCycles;
    scheduler_.ScheduleEvent(EventType::HBlank, cyclesUntilEvent);
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Window control
///---------------------------------------------------------------------------------------------------------------------------------

void PPU::SetNonObjWindowEnabled()
{
    u8 scanline = GetVCOUNT();

    // Window 1 scanline in range check
    u8 y1 = static_cast<u8>(registers_[0x47]);  // Top
    u8 y2 = static_cast<u8>(registers_[0x46]);  // Bottom

    if (scanline == y1)
    {
        window1EnabledOnScanline_ = true;
    }

    if (scanline == y2)
    {
        window1EnabledOnScanline_ = false;
    }

    // Window 0 scanline in range check
    y1 = static_cast<u8>(registers_[0x45]);  // Top
    y2 = static_cast<u8>(registers_[0x44]);  // Bottom

    if (scanline == y1)
    {
        window0EnabledOnScanline_ = true;
    }

    if (scanline == y2)
    {
        window0EnabledOnScanline_ = false;
    }
}

void PPU::ConfigureNonObjWindow(u8 leftEdge, u8 rightEdge, WindowSettings const& settings)
{
    if (rightEdge > LCD_WIDTH)
    {
        if (leftEdge >= LCD_WIDTH)
        {
            return;
        }

        rightEdge = LCD_WIDTH;
    }

    if (leftEdge <= rightEdge)
    {
        for (u8 dot = leftEdge; dot < rightEdge; ++dot)
        {
            frameBuffer_.GetWindowSettings(dot) = settings;
        }
    }
    else
    {
        for (u8 dot = 0; dot < rightEdge; ++dot)
        {
            frameBuffer_.GetWindowSettings(dot) = settings;
        }

        for (u8 dot = leftEdge; dot < LCD_WIDTH; ++dot)
        {
            frameBuffer_.GetWindowSettings(dot) = settings;
        }
    }
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Rendering
///---------------------------------------------------------------------------------------------------------------------------------

void PPU::EvaluateScanline()
{
    auto dispcnt = GetDISPCNT();
    u16 backdropColor = GetBgColor(0);

    if (!dispcnt.forceBlank)
    {
        if (dispcnt.window0Display || dispcnt.window1Display || dispcnt.objWindowDisplay)
        {
            auto winin = MemCpyInit<WININ>(&registers_[WININ::INDEX]);
            auto winout = MemCpyInit<WINOUT>(&registers_[WINOUT::INDEX]);

            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wnarrowing"
            WindowSettings outOfWindow = {
                {winout.outsideBg0Enabled, winout.outsideBg1Enabled, winout.outsideBg2Enabled, winout.outsideBg3Enabled},
                winout.outsideObjEnabled,
                winout.outsideSpecialEffect
            };
            #pragma GCC diagnostic pop

            frameBuffer_.InitializeWindow(outOfWindow);

            if (dispcnt.screenDisplayObj && dispcnt.objWindowDisplay)
            {
                #pragma GCC diagnostic push
                #pragma GCC diagnostic ignored "-Wnarrowing"
                WindowSettings objWindow = {
                    {winout.objWinBg0Enabled, winout.objWinBg1Enabled, winout.objWinBg2Enabled, winout.objWinBg3Enabled},
                    winout.objWinObjEnabled,
                    winout.objWinSpecialEffect
                };
                #pragma GCC diagnostic pop

                EvaluateOAM(&objWindow);
            }

            if (dispcnt.window1Display)
            {
                #pragma GCC diagnostic push
                #pragma GCC diagnostic ignored "-Wnarrowing"
                WindowSettings window1 = {
                    {winin.win1Bg0Enabled, winin.win1Bg1Enabled, winin.win1Bg2Enabled, winin.win1Bg3Enabled},
                    winin.win1ObjEnabled,
                    winin.win1SpecialEffect
                };
                #pragma GCC diagnostic pop

                u8 x1 = static_cast<u8>(registers_[0x43]);
                u8 x2 = static_cast<u8>(registers_[0x42]);

                if (window1EnabledOnScanline_)
                {
                    ConfigureNonObjWindow(x1, x2, window1);
                }
            }

            if (dispcnt.window0Display)
            {
                #pragma GCC diagnostic push
                #pragma GCC diagnostic ignored "-Wnarrowing"
                WindowSettings window0 = {
                    {winin.win0Bg0Enabled, winin.win0Bg1Enabled, winin.win0Bg2Enabled, winin.win0Bg3Enabled},
                    winin.win0ObjEnabled,
                    winin.win0SpecialEffect
                };
                #pragma GCC diagnostic pop

                u8 x1 = static_cast<u8>(registers_[0x41]);
                u8 x2 = static_cast<u8>(registers_[0x40]);

                if (window0EnabledOnScanline_)
                {
                    ConfigureNonObjWindow(x1, x2, window0);
                }
            }
        }
        else
        {
            WindowSettings allEnabled = {
                {true, true, true, true},
                true,
                true
            };

            frameBuffer_.InitializeWindow(allEnabled);
        }

        if (dispcnt.screenDisplayObj)
        {
            frameBuffer_.ClearSpritePixels();
            EvaluateOAM();
            frameBuffer_.PushSpritePixels();
        }

        switch (dispcnt.bgMode)
        {
            case 0:
                RenderMode0Scanline();
                break;
            case 1:
                RenderMode1Scanline();
                break;
            case 2:
                RenderMode2Scanline();
                break;
            case 3:
                RenderMode3Scanline();
                break;
            case 4:
                RenderMode4Scanline();
                break;
            default:
                break;
        }
    }

    auto bldcnt = MemCpyInit<BLDCNT>(&registers_[BLDCNT::INDEX]);
    auto bldalpha = MemCpyInit<BLDALPHA>(&registers_[BLDALPHA::INDEX]);
    auto bldy = MemCpyInit<BLDY>(&registers_[BLDY::INDEX]);

    frameBuffer_.RenderScanline(backdropColor, dispcnt.forceBlank, bldcnt, bldalpha, bldy);
    IncrementAffineBackgroundReferencePoints();
}

void PPU::RenderMode0Scanline()
{
    auto dispcnt = GetDISPCNT();

    if (dispcnt.screenDisplayBg0)
    {
        u16 xOffset = MemCpyInit<u16>(&registers_[0x10]);
        u16 yOffset = MemCpyInit<u16>(&registers_[0x12]);
        RenderRegularTiledBackgroundScanline(GetBGCNT(0), 0, xOffset & 0x01FF, yOffset & 0x01FF);
    }

    if (dispcnt.screenDisplayBg1)
    {
        u16 xOffset = MemCpyInit<u16>(&registers_[0x14]);
        u16 yOffset = MemCpyInit<u16>(&registers_[0x16]);
        RenderRegularTiledBackgroundScanline(GetBGCNT(1), 1, xOffset & 0x01FF, yOffset & 0x01FF);
    }

    if (dispcnt.screenDisplayBg2)
    {
        u16 xOffset = MemCpyInit<u16>(&registers_[0x18]);
        u16 yOffset = MemCpyInit<u16>(&registers_[0x1A]);
        RenderRegularTiledBackgroundScanline(GetBGCNT(2), 2, xOffset & 0x01FF, yOffset & 0x01FF);
    }

    if (dispcnt.screenDisplayBg3)
    {
        u16 xOffset = MemCpyInit<u16>(&registers_[0x1C]);
        u16 yOffset = MemCpyInit<u16>(&registers_[0x1E]);
        RenderRegularTiledBackgroundScanline(GetBGCNT(3), 3, xOffset & 0x01FF, yOffset & 0x01FF);
    }
}

void PPU::RenderMode1Scanline()
{
    auto dispcnt = GetDISPCNT();

    if (dispcnt.screenDisplayBg0)
    {
        u16 xOffset = MemCpyInit<u16>(&registers_[0x10]);
        u16 yOffset = MemCpyInit<u16>(&registers_[0x12]);
        RenderRegularTiledBackgroundScanline(GetBGCNT(0), 0, xOffset & 0x01FF, yOffset & 0x01FF);
    }

    if (dispcnt.screenDisplayBg1)
    {
        u16 xOffset = MemCpyInit<u16>(&registers_[0x14]);
        u16 yOffset = MemCpyInit<u16>(&registers_[0x16]);
        RenderRegularTiledBackgroundScanline(GetBGCNT(1), 1, xOffset & 0x01FF, yOffset & 0x01FF);
    }

    if (dispcnt.screenDisplayBg2)
    {
        i16 dx = MemCpyInit<i16>(&registers_[0x20]);
        i16 dy = MemCpyInit<i16>(&registers_[0x24]);
        RenderAffineTiledBackgroundScanline(GetBGCNT(2), 2, bg2RefX_, bg2RefY_, dx, dy);
    }
}

void PPU::RenderMode2Scanline()
{
    auto dispcnt = GetDISPCNT();

    if (dispcnt.screenDisplayBg2)
    {
        i16 dx = MemCpyInit<i16>(&registers_[0x20]);
        i16 dy = MemCpyInit<i16>(&registers_[0x24]);
        RenderAffineTiledBackgroundScanline(GetBGCNT(2), 2, bg2RefX_, bg2RefY_, dx, dy);
    }

    if (dispcnt.screenDisplayBg3)
    {
        i16 dx = MemCpyInit<i16>(&registers_[0x30]);
        i16 dy = MemCpyInit<i16>(&registers_[0x34]);
        RenderAffineTiledBackgroundScanline(GetBGCNT(3), 3, bg3RefX_, bg3RefY_, dx, dy);
    }
}

void PPU::RenderMode3Scanline()
{
    auto dispcnt = GetDISPCNT();
    auto bg2cnt = GetBGCNT(2);
    u8 scanline = GetVCOUNT();
    u8 priority = bg2cnt.priority;

    if (!dispcnt.screenDisplayBg2)
    {
        return;
    }

    size_t bitmapIndex = scanline * LCD_WIDTH * sizeof(u16);

    for (u8 dot = 0; dot < LCD_WIDTH; ++dot)
    {
        u16 color = MemCpyInit<u16>(&VRAM_[bitmapIndex]);
        frameBuffer_.PushPixel({PixelSrc::BG2, color, priority, false}, dot);
        bitmapIndex += sizeof(u16);
    }
}

void PPU::RenderMode4Scanline()
{
    auto dispcnt = GetDISPCNT();
    auto bg2cnt = GetBGCNT(2);
    u8 scanline = GetVCOUNT();
    u8 priority = bg2cnt.priority;

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
        frameBuffer_.PushPixel({PixelSrc::BG2, color, priority, transparent}, dot);
    }
}

void PPU::RenderRegularTiledBackgroundScanline(BGCNT bgcnt, u8 bgIndex, u16 xOffset, u16 yOffset)
{
    u16 width = (bgcnt.screenSize & 0b01) ? 512 : 256;
    u16 height = (bgcnt.screenSize & 0b10) ? 512 : 256;

    u16 x = xOffset % width;
    u16 y = (GetVCOUNT() + yOffset) % height;

    if (bgcnt.palette)
    {
        RenderRegular8bppBackground(bgcnt, bgIndex, x, y, width);
    }
    else
    {
        RenderRegular4bppBackground(bgcnt, bgIndex, x, y, width);
    }
}

void PPU::RenderRegular4bppBackground(BGCNT bgcnt, u8 bgIndex, u16 x, u16 y, u16 width)
{
    BackgroundCharBlockView charBlock(VRAM_, bgcnt.charBaseBlock);
    RegularScreenBlockScanlineView screenBlock(*this, bgcnt.screenBaseBlock, x, y, width);
    CharBlockEntry4 charBlockEntry;
    charBlock.GetCharBlock(charBlockEntry, screenBlock.TileIndex());

    auto src = static_cast<PixelSrc>(bgIndex + 1);
    u8 priority = bgcnt.priority;

    for (u8 dot = 0; dot < LCD_WIDTH; ++dot)
    {
        if (frameBuffer_.GetWindowSettings(dot).bgEnabled[bgIndex])
        {
            u8 tileX = screenBlock.TileX();
            u8 tileY = screenBlock.TileY();
            bool left = (tileX % 2) == 0;

            auto colors = charBlockEntry.pixels[tileY][tileX / 2];
            u8 colorIndex = left ? colors.leftColorIndex : colors.rightColorIndex;
            bool transparent = colorIndex == 0;
            u16 bgr555 = transparent ? GetBgColor(0) : GetBgColor(screenBlock.Palette(), colorIndex);

            frameBuffer_.PushPixel({src, bgr555, priority, transparent}, dot);
        }

        if (screenBlock.Update())
        {
            charBlock.GetCharBlock(charBlockEntry, screenBlock.TileIndex());
        }
    }
}

void PPU::RenderRegular8bppBackground(BGCNT bgcnt, u8 bgIndex, u16 x, u16 y, u16 width)
{
    BackgroundCharBlockView charBlock(VRAM_, bgcnt.charBaseBlock);
    RegularScreenBlockScanlineView screenBlock(*this, bgcnt.screenBaseBlock, x, y, width);
    CharBlockEntry8 charBlockEntry;
    charBlock.GetCharBlock(charBlockEntry, screenBlock.TileIndex());

    auto src = static_cast<PixelSrc>(bgIndex + 1);
    u8 priority = bgcnt.priority;

    for (u8 dot = 0; dot < LCD_WIDTH; ++dot)
    {
        if (frameBuffer_.GetWindowSettings(dot).bgEnabled[bgIndex])
        {
            u8 tileX = screenBlock.TileX();
            u8 tileY = screenBlock.TileY();

            u8 paletteIndex = charBlockEntry.pixels[tileY][tileX];
            bool transparent = paletteIndex == 0;
            u16 bgr555 = GetBgColor(paletteIndex);

            frameBuffer_.PushPixel({src, bgr555, priority, transparent}, dot);
        }

        if (screenBlock.Update())
        {
            charBlock.GetCharBlock(charBlockEntry, screenBlock.TileIndex());
        }
    }
}

void PPU::RenderAffineTiledBackgroundScanline(BGCNT bgcnt, u8 bgIndex, i32 x, i32 y, i16 dx, i16 dy)
{
    BackgroundCharBlockView charBlock(VRAM_, bgcnt.charBaseBlock);
    u8 mapWidthTiles;

    switch (bgcnt.screenSize)
    {
        case 0:
            mapWidthTiles = 16;
            break;
        case 1:
            mapWidthTiles = 32;
            break;
        case 2:
            mapWidthTiles = 64;
            break;
        case 3:
            mapWidthTiles = 128;
            break;
    }

    u16 mapWidthPixels = mapWidthTiles * 8;

    auto src = static_cast<PixelSrc>(bgIndex + 1);
    u8 priority = bgcnt.priority;
    bool wrap = bgcnt.wrapAround;

    size_t baseAddr = bgcnt.screenBaseBlock * SCREEN_BLOCK_SIZE;
    size_t screenBlockSize = mapWidthTiles * mapWidthTiles;
    size_t maxAvailableSpace = (32 * SCREEN_BLOCK_SIZE) - baseAddr;
    screenBlockSize = std::min(screenBlockSize, maxAvailableSpace);
    auto screenBlock = std::span<const std::byte>(&VRAM_[baseAddr], screenBlockSize);

    for (u8 dot = 0; dot < LCD_WIDTH; ++dot)
    {
        if (frameBuffer_.GetWindowSettings(dot).bgEnabled[bgIndex])
        {
            i32 screenX = x >> 8;
            i32 screenY = y >> 8;
            bool oob = (screenX < 0) || (screenX >= mapWidthPixels) || (screenY < 0) || (screenY >= mapWidthPixels);
            bool valid = wrap || !oob;
            u8 paletteIndex = 0;

            if (valid)
            {
                if (oob)
                {
                    screenX = ((screenX % mapWidthPixels) + mapWidthPixels) % mapWidthPixels;
                    screenY = ((screenY % mapWidthPixels) + mapWidthPixels) % mapWidthPixels;
                }

                u32 mapX = screenX / 8;
                u32 mapY = screenY / 8;
                u8 tileX = screenX % 8;
                u8 tileY = screenY % 8;
                u32 screenBlockIndex = mapX + (mapY * mapWidthTiles);

                if (screenBlockIndex < screenBlock.size())
                {
                    u8 tileIndex = static_cast<u8>(screenBlock[screenBlockIndex]);
                    paletteIndex = charBlock.GetAffinePaletteIndex(tileIndex, tileX, tileY);
                }
            }

            bool transparent = paletteIndex == 0;
            u16 bgr555 = GetBgColor(paletteIndex);
            frameBuffer_.PushPixel({src, bgr555, priority, transparent}, dot);
        }

        x += dx;
        y += dy;
    }
}

void PPU::EvaluateOAM(WindowSettings* windowSettingsPtr)
{
    Oam oam(reinterpret_cast<const OamEntry*>(OAM_.data()), 128);
    bool windowEval = windowSettingsPtr != nullptr;
    u8 scanline = GetVCOUNT();
    auto dispcnt = GetDISPCNT();

    for (u8 i = 0; i < 128; ++i)
    {
        OamEntry const& entry = oam[i];

        if ((windowEval && (entry.attribute0.gfxMode != 2)) ||      // Skip window sprites if not evaluating window
            (!windowEval && (entry.attribute0.gfxMode == 2)) ||     // Skip visible sprites if evaluating window
            (entry.attribute0.objMode == 2) ||                      // Skip disabled sprites
            (entry.attribute0.gfxMode == 3))                        // Skip illegal gfx mode
        {
            continue;
        }

        u8 height;
        u8 width;
        u8 dimensions = (entry.attribute0.shape << 2) | entry.attribute1.size;

        switch (dimensions)
        {
            // Square
            case 0b0000:
                height = 8;
                width = 8;
                break;
            case 0b0001:
                height = 16;
                width = 16;
                break;
            case 0b0010:
                height = 32;
                width = 32;
                break;
            case 0b0011:
                height = 64;
                width = 64;
                break;

            // Horizontal
            case 0b0100:
                height = 8;
                width = 16;
                break;
            case 0b0101:
                height = 8;
                width = 32;
                break;
            case 0b0110:
                height = 16;
                width = 32;
                break;
            case 0b0111:
                height = 32;
                width = 64;
                break;

            // Vertical
            case 0b1000:
                height = 16;
                width = 8;
                break;
            case 0b1001:
                height = 32;
                width = 8;
                break;
            case 0b1010:
                height = 32;
                width = 16;
                break;
            case 0b1011:
                height = 64;
                width = 32;
                break;

            // Illegal combinations
            default:
                continue;
        }

        i16 y = entry.attribute0.y;
        i16 x = entry.attribute1.x;

        if (y >= 160)
        {
            y -= 256;
        }

        if (x & 0x0100)
        {
            x = (~0x01FF) | (x & 0x01FF);
        }

        i16 topEdge = y;
        i16 bottomEdge = y + height - 1;

        if (entry.attribute0.objMode == 3)
        {
            y += (height / 2);
            x += (width / 2);
            topEdge = y - (height / 2);
            bottomEdge = topEdge + (2 * height) - 1;
        }

        if ((topEdge > scanline) || (scanline > bottomEdge))
        {
            continue;
        }

        if (entry.attribute0.objMode == 0)
        {
            RenderRegSprite(dispcnt.objCharacterVramMapping, x, y, width, height, entry, windowSettingsPtr);
        }
        else
        {
            RenderAffSprite(dispcnt.objCharacterVramMapping, x, y, width, height, entry, windowSettingsPtr);
        }
    }
}

void PPU::RenderRegSprite(bool oneDim, i16 x, i16 y, u8 width, u8 height, OamEntry const& entry, WindowSettings* windowSettingsPtr)
{
    i16 leftEdge = std::max(static_cast<i16>(0), x);
    i16 rightEdge = std::min(static_cast<i16>(239), static_cast<i16>(x + width - 1));
    i16 horizontalOffset = leftEdge - x;
    i16 verticalOffset = GetVCOUNT() - y;
    i16 dot = leftEdge;

    if ((horizontalOffset >= width) || (leftEdge > rightEdge))
    {
        return;
    }

    SpriteRow colors;
    ObjSpan objCharBlocks(&VRAM_[OBJ_CHAR_BLOCK_BASE_ADDR], OBJ_CHAR_BLOCKS_SIZE);

    if (oneDim)
    {
        Populate1dRegularSpriteRow(objCharBlocks, colors, entry, width, height, verticalOffset);
    }
    else
    {
        Populate2dRegularSpriteRow(objCharBlocks, colors, entry, width, height, verticalOffset);
    }

    u8 onScreenPixels = rightEdge - leftEdge + 1;
    std::span<u8> colorIndexes(&colors[horizontalOffset], onScreenPixels);

    u8 palette = entry.attribute2.palette;
    u8 priority = entry.attribute2.priority;
    bool colorMode = entry.attribute0.colorMode;
    bool semiTransparent = entry.attribute0.gfxMode == 1;

    for (u8 i = 0; i < colorIndexes.size(); ++i)
    {
        u8 colorIndex = colorIndexes[i];
        u16 bgr555 = colorMode ? GetSpriteColor(colorIndex) : GetSpriteColor(palette, colorIndex);
        bool transparent = colorIndex == 0;
        PushSpritePixel(dot++, bgr555, priority, transparent, semiTransparent, windowSettingsPtr);
    }
}

void PPU::RenderAffSprite(bool oneDim, i16 x, i16 y, u8 width, u8 height, OamEntry const& entry, WindowSettings* windowSettingsPtr)
{
    // Matrix parameters
    AffineMatrices matrices(reinterpret_cast<AffineMatrix*>(&OAM_[0]), AFFINE_MATRIX_COUNT);
    AffineMatrix const& matrix = matrices[entry.attribute1.paramSelect];

    // Bounds
    u8 widthTiles = width / 8;
    i16 leftEdge = x;
    i16 rightEdge = x + width - 1;
    i16 topEdge = y;
    u8 halfWidth = width / 2;
    u8 halfHeight = height / 2;
    bool doubleSize = entry.attribute0.objMode == 3;

    if (doubleSize)
    {
        leftEdge -= halfWidth;
        rightEdge += halfWidth;
        topEdge -= halfHeight;
    }

    // Rotation center
    i16 x0 = doubleSize ? width : halfWidth;
    i16 y0 = doubleSize ? height : halfHeight;

    // Screen/affine positions
    i16 x1 = 0;
    i16 y1 = GetVCOUNT() - topEdge;

    i32 affineX = (matrix.pa * (x1 - x0)) + (matrix.pb * (y1 - y0)) + (halfWidth << 8);
    i32 affineY = (matrix.pc * (x1 - x0)) + (matrix.pd * (y1 - y0)) + (halfHeight << 8);

    // OAM entry parameters
    u8 palette = entry.attribute2.palette;
    u8 priority = entry.attribute2.priority;
    u16 baseTile = entry.attribute2.tile;
    bool colorMode = entry.attribute0.colorMode;
    bool semiTransparent = entry.attribute0.gfxMode == 1;

    // VRAM
    ObjSpan objCharBlocks(&VRAM_[OBJ_CHAR_BLOCK_BASE_ADDR], OBJ_CHAR_BLOCKS_SIZE);

    for (i16 dot = leftEdge; (dot <= rightEdge) && (dot < LCD_WIDTH); ++dot)
    {
        i32 textureX = affineX >> 8;
        i32 textureY = affineY >> 8;
        affineX += matrix.pa;
        affineY += matrix.pc;

        if ((dot < 0) || (textureX < 0) || (textureX >= width) || (textureY < 0) || (textureY >= height))
        {
            continue;
        }

        u8 colorIndex = oneDim ?
            Get1dAffineColorIndex(objCharBlocks, baseTile, textureX, textureY, widthTiles, colorMode) :
            Get2dAffineColorIndex(objCharBlocks, baseTile, textureX, textureY, colorMode);

        u16 bgr555 = colorMode ? GetSpriteColor(colorIndex) : GetSpriteColor(palette, colorIndex);
        bool transparent = colorIndex == 0;
        PushSpritePixel(dot, bgr555, priority, transparent, semiTransparent, windowSettingsPtr);
    }
}

void PPU::PushSpritePixel(u8 dot, u16 color, u8 priority, bool transparent, bool semiTransparent, WindowSettings* windowSettingsPtr)
{
    if (windowSettingsPtr == nullptr)
    {
        // Visible Sprite
        Pixel& currentPixel = frameBuffer_.GetSpritePixel(dot);

        if (frameBuffer_.GetWindowSettings(dot).objEnabled && !transparent &&
            (!currentPixel.initialized || (priority < currentPixel.priority) || currentPixel.transparent))
        {
            currentPixel = Pixel(PixelSrc::OBJ, color, priority, transparent, semiTransparent);
        }
    }
    else if (!transparent)
    {
        // Opaque OBJ window sprite pixel
        frameBuffer_.GetWindowSettings(dot) = *windowSettingsPtr;
    }
}
}  // namespace graphics
