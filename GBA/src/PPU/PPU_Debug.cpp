#include <GBA/include/PPU/PPU.hpp>
#include <span>
#include <vector>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/PPU/Registers.hpp>
#include <GBA/include/PPU/VramViews.hpp>
#include <GBA/include/Types/DebugTypes.hpp>
#include <GBA/include/Types/Types.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>

namespace
{
/// @brief Convert fixed point affine background reference point to floating point value.
/// @param ref Fixed point value.
/// @param width Width of map in pixels.
/// @return Floating point representation of reference point.
float CalculateReferencePoint(i32 ref, u16 width)
{
    u8 fractionPortion = ref & U8_MAX;
    float val = static_cast<float>(fractionPortion) / 256.0f;

    u32 integerPortion = ((ref & 0x07FF'FF00) >> 8) % width;
    val += static_cast<float>(integerPortion);

    if (ref & U32_MSB)
    {
        val *= -1;
    }

    return val;
}

/// @brief Convert fixed point affine background scaling parameter to floating point value.
/// @param ref Fixed point value.
/// @return Floating point representation of scaling parameter.
float CalculateScalingParameter(i16 ref)
{
    u8 fractionPortion = ref & U8_MAX;
    float val = static_cast<float>(fractionPortion) / 256.0f;

    u8 integerPortion = (ref & 0x7F00) >> 8;
    val += static_cast<float>(integerPortion);

    if (ref & U16_MSB)
    {
        val *= -1;
    }

    return val;
}
}

namespace graphics
{
using namespace debug::graphics;

BackgroundDebugInfo PPU::GetBackgroundDebugInfo(u8 bgIndex) const
{
    auto dispcnt = GetDISPCNT();
    auto bgcnt = GetBGCNT(bgIndex);
    BackgroundDebugInfo debugInfo = {};
    debugInfo.priority = bgcnt.priority;
    debugInfo.mapBaseAddr = VRAM_ADDR_MIN + (bgcnt.screenBaseBlock * SCREEN_BLOCK_SIZE);
    debugInfo.tileBaseAddr = VRAM_ADDR_MIN + (bgcnt.charBaseBlock * CHAR_BLOCK_SIZE);

    if ((dispcnt.bgMode == 0) || ((dispcnt.bgMode == 1) && (bgIndex < 2)))
    {
        DebugRenderRegularBackground(bgIndex, bgcnt, debugInfo);
    }
    else if ((dispcnt.bgMode == 2) || ((dispcnt.bgMode == 1) && (bgIndex == 2)))
    {
        DebugRenderAffineBackground(bgIndex, bgcnt, debugInfo);
    }
    else if ((dispcnt.bgMode == 3) && (bgIndex == 2))
    {
        DebugRenderMode3Background(debugInfo);
    }
    else if ((dispcnt.bgMode == 4) && (bgIndex == 2))
    {
        DebugRenderMode4Background(dispcnt.displayFrameSelect, debugInfo);
    }
    else
    {
        DebugRenderRegularBackground(bgIndex, bgcnt, debugInfo);
    }

    return debugInfo;
}

void PPU::DebugRenderRegularBackground(u8 bgIndex, BGCNT bgcnt, BackgroundDebugInfo& debugInfo) const
{
    debugInfo.regular = true;
    debugInfo.width = (bgcnt.screenSize & 0b01) ? 512 : 256;
    debugInfo.height = (bgcnt.screenSize & 0b10) ? 512 : 256;
    debugInfo.buffer.resize(debugInfo.width * debugInfo.height);
    debugInfo.xOffset = MemCpyInit<u16>(&registers_[0x10 + (4 * bgIndex)]) & 0x01FF;
    debugInfo.yOffset = MemCpyInit<u16>(&registers_[0x12 + (4 * bgIndex)]) & 0x01FF;
    u8 screenBlockIndex = bgcnt.screenBaseBlock;

    DebugRenderRegularScreenBlock(bgcnt, screenBlockIndex, 0, debugInfo);

    if ((debugInfo.width == 512) && (debugInfo.height == 256))
    {
        DebugRenderRegularScreenBlock(bgcnt, screenBlockIndex + 1, 256, debugInfo);
    }
    else if ((debugInfo.width == 256) && (debugInfo.height == 512))
    {
        DebugRenderRegularScreenBlock(bgcnt, screenBlockIndex + 1, 65536, debugInfo);
    }
    else if ((debugInfo.width == 512) && (debugInfo.height == 512))
    {
        DebugRenderRegularScreenBlock(bgcnt, screenBlockIndex + 1, 256, debugInfo);
        DebugRenderRegularScreenBlock(bgcnt, screenBlockIndex + 2, 131072, debugInfo);
        DebugRenderRegularScreenBlock(bgcnt, screenBlockIndex + 3, 131328, debugInfo);
    }
}

void PPU::DebugRenderRegularScreenBlock(BGCNT bgcnt, u8 screenBlockIndex, u32 bufferBaseIndex, BackgroundDebugInfo& debugInfo) const
{
    auto screenBlockEntryPtr = reinterpret_cast<const ScreenBlockEntry*>(&VRAM_[screenBlockIndex * SCREEN_BLOCK_SIZE]);
    BackgroundCharBlockView charBlock(VRAM_, bgcnt.charBaseBlock);

    for (u8 row = 0; row < 32; ++row)
    {
        for (u8 col = 0; col < 32; ++col)
        {
            u32 insertIndex = bufferBaseIndex + (row * debugInfo.width * 8) + (col * 8);
            u16 tile = screenBlockEntryPtr->tile;
            bool hFlip = screenBlockEntryPtr->horizontalFlip;
            bool vFlip = screenBlockEntryPtr->verticalFlip;

            if (bgcnt.palette)  // 8bpp
            {
                CharBlockEntry8 charBlockEntry;
                charBlock.GetCharBlock(charBlockEntry, tile);

                for (u8 i = 0; i < 8; ++i)
                {
                    u8 charBlockEntryRow = vFlip ? (i ^ 7) : i;

                    for (u8 j = 0; j < 8; ++j)
                    {
                        u8 charBlockEntryCol = hFlip ? (j ^ 7) : j;
                        u8 paletteIndex  = charBlockEntry.pixels[charBlockEntryRow][charBlockEntryCol];
                        u16 bgr555 = GetBgColor(paletteIndex);
                        debugInfo.buffer[insertIndex + j] = bgr555;
                    }

                    insertIndex += debugInfo.width;
                }
            }
            else  // 4bpp
            {
                CharBlockEntry4 charBlockEntry;
                charBlock.GetCharBlock(charBlockEntry, tile);
                u8 palette = screenBlockEntryPtr->palette;

                for (u8 i = 0; i < 8; ++i)
                {
                    u8 charBlockEntryRow = vFlip ? (i ^ 7) : i;

                    for (u8 j = 0; j < 8; ++j)
                    {
                        u8 charBlockEntryCol = hFlip ? (j ^ 7) : j;
                        u8 paletteIndex  = (charBlockEntryCol % 2 == 0) ?
                            charBlockEntry.pixels[charBlockEntryRow][charBlockEntryCol / 2].leftColorIndex :
                            charBlockEntry.pixels[charBlockEntryRow][charBlockEntryCol / 2].rightColorIndex;

                        u16 bgr555 = GetBgColor(palette, paletteIndex);
                        debugInfo.buffer[insertIndex + j] = bgr555;
                    }

                    insertIndex += debugInfo.width;
                }
            }

            ++screenBlockEntryPtr;
        }
    }
}

void PPU::DebugRenderAffineBackground(u8 bgIndex, BGCNT bgcnt, BackgroundDebugInfo& debugInfo) const
{
    debugInfo.regular = false;
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

    debugInfo.width = mapWidthTiles * 8;
    debugInfo.height = mapWidthTiles * 8;
    debugInfo.buffer.resize(debugInfo.width * debugInfo.height);
    BackgroundCharBlockView charBlock(VRAM_, bgcnt.charBaseBlock);
    auto tilePtr = reinterpret_cast<const u8*>(&VRAM_[bgcnt.screenBaseBlock * SCREEN_BLOCK_SIZE]);

    if (bgIndex == 2)
    {
        debugInfo.refX = CalculateReferencePoint(SignExtend<i32, 27>(MemCpyInit<i32>(&registers_[0x28])), debugInfo.width);
        debugInfo.refY = CalculateReferencePoint(SignExtend<i32, 27>(MemCpyInit<i32>(&registers_[0x2C])), debugInfo.width);
        debugInfo.pa = CalculateScalingParameter(MemCpyInit<i16>(&registers_[0x20]));
        debugInfo.pb = CalculateScalingParameter(MemCpyInit<i16>(&registers_[0x22]));
        debugInfo.pc = CalculateScalingParameter(MemCpyInit<i16>(&registers_[0x24]));
        debugInfo.pd = CalculateScalingParameter(MemCpyInit<i16>(&registers_[0x26]));
    }
    else
    {
        debugInfo.refX = CalculateReferencePoint(SignExtend<i32, 27>(MemCpyInit<i32>(&registers_[0x38])), debugInfo.width);
        debugInfo.refY = CalculateReferencePoint(SignExtend<i32, 27>(MemCpyInit<i32>(&registers_[0x3C])), debugInfo.width);
        debugInfo.pa = CalculateScalingParameter(MemCpyInit<i16>(&registers_[0x30]));
        debugInfo.pb = CalculateScalingParameter(MemCpyInit<i16>(&registers_[0x32]));
        debugInfo.pc = CalculateScalingParameter(MemCpyInit<i16>(&registers_[0x34]));
        debugInfo.pd = CalculateScalingParameter(MemCpyInit<i16>(&registers_[0x36]));
    }

    for (u8 row = 0; row < mapWidthTiles; ++row)
    {
        for (u8 col = 0; col < mapWidthTiles; ++col)
        {
            u32 insertIndex = (row * debugInfo.width * 8) + (col * 8);
            CharBlockEntry8 charBlockEntry;
            charBlock.GetCharBlock(charBlockEntry, *tilePtr);

            for (u8 charBlockEntryRow = 0; charBlockEntryRow < 8; ++charBlockEntryRow)
            {
                for (u8 charBlockEntryCol = 0; charBlockEntryCol < 8; ++charBlockEntryCol)
                {
                    u8 paletteIndex  = charBlockEntry.pixels[charBlockEntryRow][charBlockEntryCol];
                    u16 bgr555 = GetBgColor(paletteIndex);
                    debugInfo.buffer[insertIndex + charBlockEntryCol] = bgr555;
                }

                insertIndex += debugInfo.width;
            }

            ++tilePtr;
        }
    }
}

void PPU::DebugRenderMode3Background(BackgroundDebugInfo& debugInfo) const
{
    debugInfo.regular = true;
    auto* pixelPtr = reinterpret_cast<const u16*>(VRAM_.data());
    debugInfo.width = 240;
    debugInfo.height = 160;
    debugInfo.buffer.resize(240 * 160);
    debugInfo.xOffset = 0;
    debugInfo.yOffset = 0;

    for (u16& pixel : debugInfo.buffer)
    {
        pixel = *pixelPtr;
        ++pixelPtr;
    }
}

void PPU::DebugRenderMode4Background(bool frameSelect, BackgroundDebugInfo& debugInfo) const
{
    debugInfo.regular = true;
    u32 bitmapIndex = frameSelect ? 0xA000 : 0;
    auto pixelPtr = reinterpret_cast<const u8*>(&VRAM_[bitmapIndex]);
    debugInfo.width = 240;
    debugInfo.height = 160;
    debugInfo.buffer.resize(240 * 160);
    debugInfo.xOffset = 0;
    debugInfo.yOffset = 0;

    for (u16& pixel : debugInfo.buffer)
    {
        pixel = GetBgColor(*pixelPtr);
        ++pixelPtr;
    }
}

u32 PPU::DebugReadRegister(u32 addr, AccessSize length)
{
    return ReadMemoryBlock(registers_, addr, LCD_IO_ADDR_MIN, length);
}
}  // namespace graphics
