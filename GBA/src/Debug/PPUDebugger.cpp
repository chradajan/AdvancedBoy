#include <GBA/include/Debug/PPUDebugger.hpp>
#include <span>
#include <GBA/include/Debug/DebugTypes.hpp>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/PPU/PPU.hpp>
#include <GBA/include/PPU/VramViews.hpp>
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

/// @brief Convert fixed point affine scaling parameter to floating point value.
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

inline u32 BGR555ToARGB32(u16 bgr555, bool transparent)
{
    u8 r = bgr555 & 0x001F;
    r = (r << 3) | (r >> 2);

    u8 g = (bgr555 & 0x03E0) >> 5;
    g = (g << 3) | (g >> 2);

    u8 b = (bgr555 & 0x7C00) >> 10;
    b = (b << 3) | (b >> 2);

    u8 a = transparent ? 0 : U8_MAX;

    return (a << 24) | (r << 16) | (g << 8) | b;
}
}  // namespace

namespace debug
{
u32 PPUDebugger::ReadRegister(u32 addr, AccessSize length) const
{
    return ReadMemoryBlock(ppu_.registers_, addr, LCD_IO_ADDR_MIN, length);
}

void PPUDebugger::GetBackgroundDebugInfo(BackgroundDebugInfo& debugInfo, u8 bgIndex) const
{
    auto dispcnt = ppu_.GetDISPCNT();
    auto bgcnt = ppu_.GetBGCNT(bgIndex);
    debugInfo.priority = bgcnt.priority;
    debugInfo.mapBaseAddr = VRAM_ADDR_MIN + (bgcnt.screenBaseBlock * SCREEN_BLOCK_SIZE);
    debugInfo.tileBaseAddr = VRAM_ADDR_MIN + (bgcnt.charBaseBlock * CHAR_BLOCK_SIZE);

    if ((dispcnt.bgMode == 0) || ((dispcnt.bgMode == 1) && (bgIndex < 2)))
    {
       RenderRegularBackground(bgIndex, bgcnt, debugInfo);
    }
    else if ((dispcnt.bgMode == 2) || ((dispcnt.bgMode == 1) && (bgIndex == 2)))
    {
       RenderAffineBackground(bgIndex, bgcnt, debugInfo);
    }
    else if ((dispcnt.bgMode == 3) && (bgIndex == 2))
    {
       RenderMode3Background(debugInfo);
    }
    else if ((dispcnt.bgMode == 4) && (bgIndex == 2))
    {
       RenderMode4Background(dispcnt.displayFrameSelect, debugInfo);
    }
    else
    {
       RenderRegularBackground(bgIndex, bgcnt, debugInfo);
    }
}

void PPUDebugger::RenderRegularBackground(u8 bgIndex, BGCNT bgcnt, BackgroundDebugInfo& debugInfo) const
{
    debugInfo.regular = true;
    debugInfo.width = (bgcnt.screenSize & 0b01) ? 512 : 256;
    debugInfo.height = (bgcnt.screenSize & 0b10) ? 512 : 256;
    debugInfo.xOffset = MemCpyInit<u16>(&ppu_.registers_[0x10 + (4 * bgIndex)]) & 0x01FF;
    debugInfo.yOffset = MemCpyInit<u16>(&ppu_.registers_[0x12 + (4 * bgIndex)]) & 0x01FF;
    u8 screenBlockIndex = bgcnt.screenBaseBlock;

    RenderRegularScreenBlock(bgcnt, screenBlockIndex, 0, debugInfo);

    if ((debugInfo.width == 512) && (debugInfo.height == 256))
    {
        RenderRegularScreenBlock(bgcnt, screenBlockIndex + 1, 256, debugInfo);
    }
    else if ((debugInfo.width == 256) && (debugInfo.height == 512))
    {
        RenderRegularScreenBlock(bgcnt, screenBlockIndex + 1, 65536, debugInfo);
    }
    else if ((debugInfo.width == 512) && (debugInfo.height == 512))
    {
        RenderRegularScreenBlock(bgcnt, screenBlockIndex + 1, 256, debugInfo);
        RenderRegularScreenBlock(bgcnt, screenBlockIndex + 2, 131072, debugInfo);
        RenderRegularScreenBlock(bgcnt, screenBlockIndex + 3, 131328, debugInfo);
    }
}

void PPUDebugger::RenderRegularScreenBlock(BGCNT bgcnt,
                                           u8 screenBlockIndex,
                                           u32 bufferBaseIndex,
                                           BackgroundDebugInfo& debugInfo) const
{
    auto screenBlockEntryPtr = reinterpret_cast<const ScreenBlockEntry*>(&ppu_.VRAM_[screenBlockIndex * SCREEN_BLOCK_SIZE]);
    BackgroundCharBlockView charBlock(ppu_.VRAM_, bgcnt.charBaseBlock);

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
                        u16 bgr555 = ppu_.GetBgColor(paletteIndex);
                        bool transparent = paletteIndex == 0;
                        debugInfo.buffer[insertIndex + j] = BGR555ToARGB32(bgr555, transparent);;
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

                        u16 bgr555 = ppu_.GetBgColor(palette, paletteIndex);
                        bool transparent = paletteIndex == 0;
                        debugInfo.buffer[insertIndex + j] = BGR555ToARGB32(bgr555, transparent);
                    }

                    insertIndex += debugInfo.width;
                }
            }

            ++screenBlockEntryPtr;
        }
    }
}

void PPUDebugger::RenderAffineBackground(u8 bgIndex, BGCNT bgcnt, BackgroundDebugInfo& debugInfo) const
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
    BackgroundCharBlockView charBlock(ppu_.VRAM_, bgcnt.charBaseBlock);
    auto tilePtr = reinterpret_cast<const u8*>(&ppu_.VRAM_[bgcnt.screenBaseBlock * SCREEN_BLOCK_SIZE]);

    if (bgIndex == 2)
    {
        debugInfo.refX = CalculateReferencePoint(SignExtend<i32, 27>(MemCpyInit<i32>(&ppu_.registers_[0x28])), debugInfo.width);
        debugInfo.refY = CalculateReferencePoint(SignExtend<i32, 27>(MemCpyInit<i32>(&ppu_.registers_[0x2C])), debugInfo.width);
        debugInfo.pa = CalculateScalingParameter(MemCpyInit<i16>(&ppu_.registers_[0x20]));
        debugInfo.pb = CalculateScalingParameter(MemCpyInit<i16>(&ppu_.registers_[0x22]));
        debugInfo.pc = CalculateScalingParameter(MemCpyInit<i16>(&ppu_.registers_[0x24]));
        debugInfo.pd = CalculateScalingParameter(MemCpyInit<i16>(&ppu_.registers_[0x26]));
    }
    else
    {
        debugInfo.refX = CalculateReferencePoint(SignExtend<i32, 27>(MemCpyInit<i32>(&ppu_.registers_[0x38])), debugInfo.width);
        debugInfo.refY = CalculateReferencePoint(SignExtend<i32, 27>(MemCpyInit<i32>(&ppu_.registers_[0x3C])), debugInfo.width);
        debugInfo.pa = CalculateScalingParameter(MemCpyInit<i16>(&ppu_.registers_[0x30]));
        debugInfo.pb = CalculateScalingParameter(MemCpyInit<i16>(&ppu_.registers_[0x32]));
        debugInfo.pc = CalculateScalingParameter(MemCpyInit<i16>(&ppu_.registers_[0x34]));
        debugInfo.pd = CalculateScalingParameter(MemCpyInit<i16>(&ppu_.registers_[0x36]));
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
                    u16 bgr555 = ppu_.GetBgColor(paletteIndex);
                    bool transparent = paletteIndex == 0;
                    debugInfo.buffer[insertIndex + charBlockEntryCol] = BGR555ToARGB32(bgr555, transparent);
                }

                insertIndex += debugInfo.width;
            }

            ++tilePtr;
        }
    }
}

void PPUDebugger::RenderMode3Background(BackgroundDebugInfo& debugInfo) const
{
    debugInfo.regular = true;
    auto* pixelPtr = reinterpret_cast<const u16*>(ppu_.VRAM_.data());
    debugInfo.width = 240;
    debugInfo.height = 160;
    debugInfo.xOffset = 0;
    debugInfo.yOffset = 0;

    for (u32 i = 0; i < 240 * 160; ++i)
    {
        debugInfo.buffer[i] = BGR555ToARGB32(*pixelPtr, false);
        ++pixelPtr;
    }
}

void PPUDebugger::RenderMode4Background(bool frameSelect, BackgroundDebugInfo& debugInfo) const
{
    debugInfo.regular = true;
    u32 bitmapIndex = frameSelect ? 0xA000 : 0;
    auto pixelPtr = reinterpret_cast<const u8*>(&ppu_.VRAM_[bitmapIndex]);
    debugInfo.width = 240;
    debugInfo.height = 160;
    debugInfo.xOffset = 0;
    debugInfo.yOffset = 0;

    for (u32 i = 0; i < 240 * 160; ++i)
    {
        u16 bgr555 = ppu_.GetBgColor(*pixelPtr);
        bool transparent = *pixelPtr == 0;
        debugInfo.buffer[i] = BGR555ToARGB32(bgr555, transparent);
        ++pixelPtr;
    }
}

void PPUDebugger::GetSpriteDebugInfo(SpriteDebugInfo& sprites, bool regTransforms, bool affTransforms) const
{
    Oam oam(reinterpret_cast<const OamEntry*>(ppu_.OAM_.data()), 128);
    AffineMatrices matrices(reinterpret_cast<const AffineMatrix*>(ppu_.OAM_.data()), AFFINE_MATRIX_COUNT);
    SpriteRow colorIndexes;
    ObjSpan objCharBlocks(&ppu_.VRAM_[OBJ_CHAR_BLOCK_BASE_ADDR], OBJ_CHAR_BLOCKS_SIZE);
    bool oneDimMapping = ppu_.GetDISPCNT().objCharacterVramMapping;

    for (u8 i = 0; i < 128; ++i)
    {
        OamEntry const& oamEntry = oam[i];
        Sprite& debugEntry = sprites[i];

        if ((oamEntry.attribute0.objMode == 2) || (oamEntry.attribute0.gfxMode == 3))  // Skip disabled sprites and illegal gfx mode
        {
            debugEntry.enabled = false;
            continue;
        }

        u8 height;
        u8 width;
        u8 dimensions = (oamEntry.attribute0.shape << 2) | oamEntry.attribute1.size;

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
                debugEntry.enabled = false;
                continue;
        }

        debugEntry.enabled = true;
        debugEntry.width = width;
        debugEntry.height = height;
        debugEntry.x = oamEntry.attribute1.x;
        debugEntry.y = oamEntry.attribute0.y;
        debugEntry.tileIndex = oamEntry.attribute2.tile;
        debugEntry.oamIndex = i;
        debugEntry.mosaic = oamEntry.attribute0.mosaic;
        debugEntry.priority = oamEntry.attribute2.priority;

        bool colorMode = oamEntry.attribute0.colorMode;
        debugEntry.palette = colorMode ? U8_MAX : oamEntry.attribute2.palette;

        switch (oamEntry.attribute0.gfxMode)
        {
            case 0:
                debugEntry.gxfMode = "Normal";
                break;
            case 1:
                debugEntry.gxfMode = "Alpha Blend";
                break;
            case 2:
                debugEntry.gxfMode = "Window";
                break;
            default:
                debugEntry.gxfMode = "Illegal";
                break;
        }

        if (oamEntry.attribute0.objMode == 0)
        {
            debugEntry.regular = true;
            debugEntry.doubleSize = false;
            debugEntry.horizontalFlip = oamEntry.attribute1.horizontalFlip;
            debugEntry.verticalFlip = oamEntry.attribute1.verticalFlip;

            u16 bufferIndex = 0;

            for (u8 row = 0; row < height; ++row)
            {
                bool hFlip = regTransforms ? oamEntry.attribute1.horizontalFlip : false;
                bool vFlip = regTransforms ? oamEntry.attribute1.verticalFlip : false;

                if (oneDimMapping)
                {
                    Populate1dRegularSpriteRow(objCharBlocks,
                                               colorIndexes,
                                               oamEntry,
                                               width,
                                               height,
                                               row,
                                               hFlip,
                                               vFlip);
                }
                else
                {
                    Populate2dRegularSpriteRow(objCharBlocks,
                                               colorIndexes,
                                               oamEntry,
                                               width,
                                               height,
                                               row,
                                               hFlip,
                                               vFlip);
                }

                for (u8 col = 0; col < width; ++col)
                {
                    u8 colorIndex = colorIndexes[col];
                    u16 bgr555 = colorMode ? ppu_.GetSpriteColor(colorIndex) : ppu_.GetSpriteColor(debugEntry.palette, colorIndex);
                    bool transparent = colorIndex == 0;
                    debugEntry.buffer[bufferIndex++] = BGR555ToARGB32(bgr555, transparent);
                }
            }
        }
        else
        {
            debugEntry.regular = false;
            debugEntry.doubleSize = oamEntry.attribute0.objMode == 3;
            debugEntry.horizontalFlip = false;
            debugEntry.verticalFlip = false;
            debugEntry.parameterIndex = oamEntry.attribute1.paramSelect;

            AffineMatrix const& matrix = matrices[debugEntry.parameterIndex];
            debugEntry.pa = CalculateScalingParameter(matrix.pa);
            debugEntry.pb = CalculateScalingParameter(matrix.pb);
            debugEntry.pc = CalculateScalingParameter(matrix.pc);
            debugEntry.pd = CalculateScalingParameter(matrix.pd);

            u16 bufferIndex = 0;

            if (affTransforms)
            {
                // Bounds
                u8 widthTiles = width / 8;
                u8 halfWidth = width / 2;
                u8 halfHeight = height / 2;

                // Rotation center
                i16 x0 = debugEntry.doubleSize ? width : halfWidth;
                i16 y0 = debugEntry.doubleSize ? height : halfHeight;

                // Pixel
                u32 transparentPixel =
                    BGR555ToARGB32(colorMode ? ppu_.GetSpriteColor(0) : ppu_.GetSpriteColor(debugEntry.palette, 0), true);

                for (u8 row = 0; row < (debugEntry.doubleSize ? height * 2 : height); ++row)
                {
                    // Screen/affine positions
                    i16 x1 = 0;
                    i16 y1 = row;
                    i32 affineX = (matrix.pa * (x1 - x0)) + (matrix.pb * (y1 - y0)) + (halfWidth << 8);
                    i32 affineY = (matrix.pc * (x1 - x0)) + (matrix.pd * (y1 - y0)) + (halfHeight << 8);

                    for (u8 col = 0; col < (debugEntry.doubleSize ? width * 2 : width); ++col)
                    {
                        i32 textureX = affineX >> 8;
                        i32 textureY = affineY >> 8;
                        affineX += matrix.pa;
                        affineY += matrix.pc;

                        if ((textureX < 0) || (textureX >= width) || (textureY < 0) || (textureY >= height))
                        {
                            debugEntry.buffer[bufferIndex++] = transparentPixel;
                            continue;
                        }

                        u8 colorIndex = oneDimMapping ?
                            Get1dAffineColorIndex(objCharBlocks, debugEntry.tileIndex, textureX, textureY, widthTiles, colorMode) :
                            Get2dAffineColorIndex(objCharBlocks, debugEntry.tileIndex, textureX, textureY, colorMode);

                        u16 bgr555 = colorMode ? ppu_.GetSpriteColor(colorIndex) :
                                                 ppu_.GetSpriteColor(debugEntry.palette, colorIndex);
                        bool transparent = colorIndex == 0;
                        debugEntry.buffer[bufferIndex++] = BGR555ToARGB32(bgr555, transparent);
                    }
                }
            }
            else
            {
                for (u8 row = 0; row < height; ++row)
                {
                    if (oneDimMapping)
                    {
                        Populate1dRegularSpriteRow(objCharBlocks, colorIndexes, oamEntry, width, height, row, false, false);
                    }
                    else
                    {
                        Populate2dRegularSpriteRow(objCharBlocks, colorIndexes, oamEntry, width, height, row, false, false);
                    }

                    for (u8 col = 0; col < width; ++col)
                    {
                        u8 colorIndex = colorIndexes[col];
                        u16 bgr555 = colorMode ? ppu_.GetSpriteColor(colorIndex) :
                                                 ppu_.GetSpriteColor(debugEntry.palette, colorIndex);
                        bool transparent = colorIndex == 0;
                        debugEntry.buffer[bufferIndex++] = BGR555ToARGB32(bgr555, transparent);
                    }
                }
            }
        }
    }
}
}  // namespace debug
