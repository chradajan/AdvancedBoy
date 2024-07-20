#include <GBA/include/PPU/PPU.hpp>
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
#include <GBA/include/Types.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>

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

void PPU::IncrementAffineBackgroundReferencePoints()
{
    i16 delta;

    std::memcpy(&delta, &registers_[0x22], sizeof(i16));    bg2RefX_ += delta;  // PB
    std::memcpy(&delta, &registers_[0x26], sizeof(i16));    bg2RefY_ += delta;  // PD
    std::memcpy(&delta, &registers_[0x32], sizeof(i16));    bg3RefX_ += delta;  // PB
    std::memcpy(&delta, &registers_[0x36], sizeof(i16));    bg3RefY_ += delta;  // PD
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
            WININ winin;    std::memcpy(&winin, &registers_[WININ::INDEX], sizeof(WININ));
            WINOUT winout;  std::memcpy(&winout, &registers_[WINOUT::INDEX], sizeof(WINOUT));

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

                // TODO: Evaluate OAM to setup OBJ window
                (void)objWindow;
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

        switch (dispcnt.bgMode)
        {
            case 0:
                RenderMode0Scanline();
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

    BLDCNT bldcnt;      std::memcpy(&bldcnt, &registers_[BLDCNT::INDEX], sizeof(BLDCNT));
    BLDALPHA bldalpha;  std::memcpy(&bldalpha, &registers_[BLDALPHA::INDEX], sizeof(BLDALPHA));
    BLDY bldy;          std::memcpy(&bldy, &registers_[BLDY::INDEX], sizeof(BLDY));

    frameBuffer_.RenderScanline(backdropColor, dispcnt.forceBlank, bldcnt, bldalpha, bldy);
    IncrementAffineBackgroundReferencePoints();
}

void PPU::RenderMode0Scanline()
{
    auto dispcnt = GetDISPCNT();

    if (dispcnt.screenDisplayBg0)
    {
        u16 xOffset; std::memcpy(&xOffset, &registers_[0x10], sizeof(u16));
        u16 yOffset; std::memcpy(&yOffset, &registers_[0x12], sizeof(u16));
        RenderRegularTiledBackgroundScanline(GetBGCNT(0), 0, xOffset & 0x01FF, yOffset & 0x01FF);
    }

    if (dispcnt.screenDisplayBg1)
    {
        u16 xOffset; std::memcpy(&xOffset, &registers_[0x14], sizeof(u16));
        u16 yOffset; std::memcpy(&yOffset, &registers_[0x16], sizeof(u16));
        RenderRegularTiledBackgroundScanline(GetBGCNT(1), 1, xOffset & 0x01FF, yOffset & 0x01FF);
    }

    if (dispcnt.screenDisplayBg2)
    {
        u16 xOffset; std::memcpy(&xOffset, &registers_[0x18], sizeof(u16));
        u16 yOffset; std::memcpy(&yOffset, &registers_[0x1A], sizeof(u16));
        RenderRegularTiledBackgroundScanline(GetBGCNT(2), 2, xOffset & 0x01FF, yOffset & 0x01FF);
    }

    if (dispcnt.screenDisplayBg3)
    {
        u16 xOffset; std::memcpy(&xOffset, &registers_[0x1C], sizeof(u16));
        u16 yOffset; std::memcpy(&yOffset, &registers_[0x1E], sizeof(u16));
        RenderRegularTiledBackgroundScanline(GetBGCNT(3), 3, xOffset & 0x01FF, yOffset & 0x01FF);
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
        u16 color; std::memcpy(&color, &VRAM_[bitmapIndex], sizeof(u16));
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
    BackgroundCharBlockView charBlock(*this, bgcnt.charBaseBlock);
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
            u8 colorIndex = left ? colors.leftNibble : colors.rightNibble;
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
    BackgroundCharBlockView charBlock(*this, bgcnt.charBaseBlock);
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

            auto paletteIndex = charBlockEntry.pixels[tileY][tileX];
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
}  // namespace graphics
