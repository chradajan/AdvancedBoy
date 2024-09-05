#include <GBA/include/PPU/VramViews.hpp>
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstring>
#include <span>
#include <GBA/include/PPU/PPU.hpp>
#include <GBA/include/PPU/Registers.hpp>
#include <GBA/include/Types/Types.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>

namespace graphics
{
///---------------------------------------------------------------------------------------------------------------------------------
/// BackgroundCharBlockView
///---------------------------------------------------------------------------------------------------------------------------------
BackgroundCharBlockView::BackgroundCharBlockView(VramSpan vram, u8 baseIndex) :
    charBlocks_(vram.data(), BG_CHAR_BLOCKS_SIZE),
    baseIndex_(baseIndex * CHAR_BLOCK_SIZE)
{
}

void BackgroundCharBlockView::GetCharBlock(CharBlockEntry4& block, u16 index)
{
    u32 adjustedIndex = baseIndex_ + (index * sizeof(CharBlockEntry4));

    if (adjustedIndex >= charBlocks_.size())
    {
        std::memset(&block, 0, sizeof(CharBlockEntry4));
    }
    else
    {
        std::memcpy(&block, &charBlocks_[adjustedIndex], sizeof(CharBlockEntry4));
    }
}

void BackgroundCharBlockView::GetCharBlock(CharBlockEntry8& block, u16 index)
{
    u32 adjustedIndex = baseIndex_ + (index * sizeof(CharBlockEntry8));

    if (adjustedIndex >= charBlocks_.size())
    {
        std::memset(&block, 0, sizeof(CharBlockEntry8));
    }
    else
    {
        std::memcpy(&block, &charBlocks_[adjustedIndex], sizeof(CharBlockEntry8));
    }
}

u8 BackgroundCharBlockView::GetAffinePaletteIndex(u16 index, u8 tileX, u8 tileY)
{
    u32 adjustedIndex = baseIndex_ + (index * sizeof(CharBlockEntry8)) + (tileY * 8) + tileX;

    if (adjustedIndex >= charBlocks_.size())
    {
        return 0;
    }

    return static_cast<u8>(charBlocks_[adjustedIndex]);
}

///---------------------------------------------------------------------------------------------------------------------------------
/// Sprite views
///---------------------------------------------------------------------------------------------------------------------------------

void Populate1dRegularSpriteRow(ObjSpan vram, SpriteRow& colors, OamEntry const& entry, u8 width, u8 height, i16 verticalOffset)
{
    u8 widthTiles = width / 8;
    u8 heightTiles = height / 8;
    bool verticalFlip = entry.attribute1.verticalFlip;
    bool horizontalFlip = entry.attribute1.horizontalFlip;
    bool colorMode = entry.attribute0.colorMode;

    u32 baseTileOffset = verticalFlip ?
        (heightTiles - (verticalOffset / 8) - 1) * widthTiles :
        (verticalOffset / 8) * widthTiles;

    if (colorMode)
    {
        baseTileOffset *= 2;
    }

    u32 baseTile = entry.attribute2.tile + baseTileOffset;

    u32 tileY = verticalFlip ?
        (verticalOffset % 8) ^ 7 :
        verticalOffset % 8;

    u8 tileOffset = tileY * (colorMode ? CHAR_BLOCK_8_ROW_SIZE : CHAR_BLOCK_4_ROW_SIZE);
    u32 vramAddr = (baseTile * sizeof(CharBlockEntry4)) + tileOffset;
    u8 colorIndex = 0;

    for (u8 i = 0; i < widthTiles; ++i)
    {
        if (vramAddr >= OBJ_CHAR_BLOCKS_SIZE)
        {
            vramAddr -= OBJ_CHAR_BLOCKS_SIZE;
        }

        if (colorMode)
        {
            std::memcpy(&colors[colorIndex], &vram[vramAddr], CHAR_BLOCK_8_ROW_SIZE);
            colorIndex += 8;
            vramAddr += sizeof(CharBlockEntry8);
        }
        else
        {
            for (u8 j = 0; j < 4; ++j)
            {
                ColorIndexes4 indexes = std::bit_cast<ColorIndexes4>(vram[vramAddr + j]);
                colors[colorIndex++] = indexes.leftColorIndex;
                colors[colorIndex++] = indexes.rightColorIndex;
            }

            vramAddr += sizeof(CharBlockEntry4);
        }
    }

    if (horizontalFlip)
    {
        std::reverse(colors.begin(), colors.begin() + colorIndex);
    }
}

void Populate2dRegularSpriteRow(ObjSpan vram, SpriteRow& colors, OamEntry const& entry, u8 width, u8 height, i16 verticalOffset)
{
    u8 widthTiles = width / 8;
    u8 heightTiles = height / 8;
    bool colorMode = entry.attribute0.colorMode;
    bool verticalFlip = entry.attribute1.verticalFlip;
    bool horizontalFlip = entry.attribute1.horizontalFlip;
    u32 baseTile = colorMode ? (entry.attribute2.tile & ~0x01) : entry.attribute2.tile;

    baseTile += verticalFlip ?
        ((heightTiles - (verticalOffset / 8) - 1) * CHAR_BLOCK_2D_TILES_PER_ROW) :
        ((verticalOffset / 8) * CHAR_BLOCK_2D_TILES_PER_ROW);

    if (baseTile >= OBJ_CHAR_BLOCK_TILE_COUNT)
    {
        baseTile -= OBJ_CHAR_BLOCK_TILE_COUNT;
    }

    u8 row = baseTile / CHAR_BLOCK_2D_TILES_PER_ROW;
    u32 maxRowAddress = ((row + 1) * CHAR_BLOCK_2D_ROW_SIZE) - 1;

    u32 tileY = verticalFlip ?
        (verticalOffset % 8) ^ 7 :
        verticalOffset % 8;

    u8 tileOffset = tileY * (colorMode ? CHAR_BLOCK_8_ROW_SIZE : CHAR_BLOCK_4_ROW_SIZE);
    u32 vramAddr = (baseTile * sizeof(CharBlockEntry4)) + tileOffset;
    u8 colorIndex = 0;

    for (u8 i = 0; i < widthTiles; ++i)
    {
        if (vramAddr > maxRowAddress)
        {
            vramAddr -= CHAR_BLOCK_2D_ROW_SIZE;
        }

        if (colorMode)
        {
            std::memcpy(&colors[colorIndex], &vram[vramAddr], CHAR_BLOCK_8_ROW_SIZE);
            colorIndex += 8;
            vramAddr += sizeof(CharBlockEntry8);
        }
        else
        {
            for (u8 j = 0; j < 4; ++j)
            {
                ColorIndexes4 indexes = std::bit_cast<ColorIndexes4>(vram[vramAddr + j]);
                colors[colorIndex++] = indexes.leftColorIndex;
                colors[colorIndex++] = indexes.rightColorIndex;
            }

            vramAddr += sizeof(CharBlockEntry4);
        }
    }

    if (horizontalFlip)
    {
        std::reverse(colors.begin(), colors.begin() + colorIndex);
    }
}

u8 Get1dAffineColorIndex(ObjSpan vram, u16 baseTile, i32 textureX, i32 textureY, u8 widthTiles, bool colorMode)
{
    u8 horizontalTileOffset = textureX / 8;
    u8 verticalTileOffset = textureY / 8;
    u8 tileX = textureX % 8;
    u8 tileY = textureY % 8;

    u8 tileSizeBytes = colorMode ? sizeof(CharBlockEntry8) : sizeof(CharBlockEntry4);
    u8 tileRowSizeBytes = colorMode ? CHAR_BLOCK_8_ROW_SIZE : CHAR_BLOCK_4_ROW_SIZE;

    u32 tileOffset = ((verticalTileOffset * widthTiles) + horizontalTileOffset) * tileSizeBytes;
    u32 offsetWithinTile = (tileY * tileRowSizeBytes) + (colorMode ? tileX : (tileX / 2));
    u32 vramAddr = (baseTile * sizeof(CharBlockEntry4)) + tileOffset + offsetWithinTile;

    if (vramAddr >= OBJ_CHAR_BLOCKS_SIZE)
    {
        vramAddr -= OBJ_CHAR_BLOCKS_SIZE;
    }

    u8 colorIndex;

    if (colorMode)
    {
        colorIndex = static_cast<u8>(vram[vramAddr]);
    }
    else
    {
        bool left = (tileX % 2) == 0;
        ColorIndexes4 indexes = std::bit_cast<ColorIndexes4>(vram[vramAddr]);
        colorIndex = left ? indexes.leftColorIndex : indexes.rightColorIndex;
    }

    return colorIndex;
}

u8 Get2dAffineColorIndex(ObjSpan vram, u16 baseTile, i32 textureX, i32 textureY, bool colorMode)
{
    u8 horizontalTileOffset = textureX / 8;
    u8 verticalTileOffset = textureY / 8;
    u8 tileX = textureX % 8;
    u8 tileY = textureY % 8;

    if (colorMode)
    {
        baseTile &= ~0x01;
        horizontalTileOffset *= 2;
    }

    u16 tile = baseTile + (verticalTileOffset * CHAR_BLOCK_2D_TILES_PER_ROW);

    if (tile >= OBJ_CHAR_BLOCK_TILE_COUNT)
    {
        tile -= OBJ_CHAR_BLOCK_TILE_COUNT;
    }

    u8 row = tile / CHAR_BLOCK_2D_TILES_PER_ROW;
    tile += horizontalTileOffset;

    if ((tile / CHAR_BLOCK_2D_TILES_PER_ROW) != row)
    {
        tile -= CHAR_BLOCK_2D_TILES_PER_ROW;
    }

    u8 tileRowSizeBytes = colorMode ? CHAR_BLOCK_8_ROW_SIZE : CHAR_BLOCK_4_ROW_SIZE;
    u8 offsetWithinTile = (tileY * tileRowSizeBytes) + (colorMode ? tileX : (tileX / 2));
    u32 vramAddr = (tile * sizeof(CharBlockEntry4)) + offsetWithinTile;

    u8 colorIndex;

    if (colorMode)
    {
        colorIndex = static_cast<u8>(vram[vramAddr]);
    }
    else
    {
        bool left = (tileX % 2) == 0;
        ColorIndexes4 indexes = std::bit_cast<ColorIndexes4>(vram[vramAddr]);
        colorIndex = left ? indexes.leftColorIndex : indexes.rightColorIndex;
    }

    return colorIndex;
}

///---------------------------------------------------------------------------------------------------------------------------------
/// RegularScreenBlockScanlineView
///---------------------------------------------------------------------------------------------------------------------------------

RegularScreenBlockScanlineView::RegularScreenBlockScanlineView(PPU const& ppu, u8 baseIndex, u16 x, u16 y, u16 width) :
    x_(x),
    y_(y),
    screenBlockX_(x / 8),
    width_(width)
{
    // Initialize screen block views
    bool doubleWidth = width == 512;
    bool doubleHeight = y > 255;
    u8 screenBlockY = (y / 8) % 32;
    u32 screenBlockOffset = screenBlockY * SCREEN_BLOCK_SCANLINE_SIZE;
    u8 leftBaseBlockIndex = baseIndex;

    if (doubleHeight)
    {
        ++leftBaseBlockIndex;

        if (doubleWidth)
        {
            ++leftBaseBlockIndex;
        }
    }

    ScreenBlock leftScreenBlock(&ppu.VRAM_.at(leftBaseBlockIndex * SCREEN_BLOCK_SIZE), SCREEN_BLOCK_SIZE);
    left_ = std::span<const std::byte>(&leftScreenBlock[screenBlockOffset], SCREEN_BLOCK_SCANLINE_SIZE);

    if (doubleWidth)
    {
        u8 rightBaseBlockIndex = leftBaseBlockIndex + 1;
        ScreenBlock rightScreenBlock(&ppu.VRAM_.at(rightBaseBlockIndex * SCREEN_BLOCK_SIZE), SCREEN_BLOCK_SIZE);
        right_ = std::span<const std::byte>(&rightScreenBlock[screenBlockOffset], SCREEN_BLOCK_SCANLINE_SIZE);
    }
    else
    {
        right_ = left_;
    }

    // Initialize starting screen block entry data
    const std::byte* currentEntryAddr =
        (screenBlockX_ >= SCREEN_BLOCK_WIDTH) ? &right_[(screenBlockX_ - SCREEN_BLOCK_WIDTH) * sizeof(ScreenBlockEntry)] :
                                                &left_[screenBlockX_ * sizeof(ScreenBlockEntry)];

    auto currentEntry = MemCpyInit<ScreenBlockEntry>(currentEntryAddr);
    horizontalFlip_ = currentEntry.horizontalFlip;
    verticalFlip_ = currentEntry.verticalFlip;
    palette_ = currentEntry.palette;
    tile_ = currentEntry.tile;

    tileX_ = horizontalFlip_ ? (x_ % 8) ^ 7 : x_ % 8;
    tileY_ = verticalFlip_ ? (y_ % 8) ^ 7 : y_ % 8;
}

bool RegularScreenBlockScanlineView::Update()
{
    bool entryUpdated = false;
    x_ = (x_ + 1) % width_;

    if ((x_ % 8) == 0)
    {
        screenBlockX_ = (screenBlockX_ + 1) % (SCREEN_BLOCK_WIDTH * 2);

        const std::byte* currentEntryAddr =
            (screenBlockX_ >= SCREEN_BLOCK_WIDTH) ? &right_[(screenBlockX_ - SCREEN_BLOCK_WIDTH) * sizeof(ScreenBlockEntry)] :
                                                    &left_[screenBlockX_ * sizeof(ScreenBlockEntry)];

        auto currentEntry = MemCpyInit<ScreenBlockEntry>(currentEntryAddr);
        horizontalFlip_ = currentEntry.horizontalFlip;
        verticalFlip_ = currentEntry.verticalFlip;
        palette_ = currentEntry.palette;
        tile_ = currentEntry.tile;
        tileY_ = verticalFlip_ ? (y_ % 8) ^ 7 : y_ % 8;
        entryUpdated = true;
    }

    tileX_ = horizontalFlip_ ? (x_ % 8) ^ 7 : x_ % 8;
    return entryUpdated;
}
}
