#include <GBA/include/PPU/VramViews.hpp>
#include <cstddef>
#include <cstring>
#include <span>
#include <GBA/include/PPU/PPU.hpp>
#include <GBA/include/PPU/Registers.hpp>
#include <GBA/include/Types.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>

namespace graphics
{
///---------------------------------------------------------------------------------------------------------------------------------
/// BackgroundCharBlockView
///---------------------------------------------------------------------------------------------------------------------------------
BackgroundCharBlockView::BackgroundCharBlockView(PPU const& ppu, u8 baseIndex) :
    charBlocks_(ppu.VRAM_.data(), BG_CHAR_BLOCKS_SIZE),
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

void Populate1dRegularSpriteRow(VramSpan vram, SpriteRow& colors, OamEntry const& entry, u8 width, u8 height, i16 verticalOffset)
{
    u8 widthTiles = width / 8;
    u8 heightTiles = height / 8;
    bool verticalFlip = entry.attribute1.verticalFlip;

    u32 baseTileIndex = verticalFlip ?
        entry.attribute2.tile + ((heightTiles - (verticalOffset / 8) - 1) * widthTiles) :
        entry.attribute2.tile + ((verticalOffset / 8) * widthTiles);

    u32 tileY = verticalFlip ?
        (verticalOffset % 8) ^ 7 :
        verticalOffset % 8;

    bool colorMode = entry.attribute0.colorMode;
    u8 tileOffset = tileY * (colorMode ? CHAR_BLOCK_8_ROW_SIZE : CHAR_BLOCK_4_ROW_SIZE);
    u32 vramAddr = OBJ_CHAR_BLOCK_BASE_ADDR + (baseTileIndex * sizeof(CharBlockEntry4)) + tileOffset;
    u8 colorIndex = 0;

    for (u8 i = 0; i < widthTiles; ++i)
    {
        if (vramAddr >= vram.size())
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
                u8 val = static_cast<u8>(vram[vramAddr + j]);
                colors[colorIndex++] = val & 0x0F;
                colors[colorIndex++] = val >> 4;
            }

            vramAddr += sizeof(CharBlockEntry4);
        }
    }
}

void Populate2dRegularSpriteRow(VramSpan vram, SpriteRow& colors, OamEntry const& entry, u8 width, u8 height, i16 verticalOffset)
{
    u8 widthTiles = width / 8;
    u8 heightTiles = height / 8;
    bool verticalFlip = entry.attribute1.verticalFlip;

    u32 baseTileIndex = verticalFlip ?
        entry.attribute2.tile + ((heightTiles - (verticalOffset / 8) - 1) * 32) :
        entry.attribute2.tile + ((verticalOffset / 8) * 32);

    if (baseTileIndex >= OBJ_CHAR_BLOCK_TILE_COUNT)
    {
        baseTileIndex -= OBJ_CHAR_BLOCK_TILE_COUNT;
    }

    u8 row = baseTileIndex / CHAR_BLOCK_2D_ROW_WIDTH;
    u32 maxRowAddress = OBJ_CHAR_BLOCK_BASE_ADDR + ((row + 1) * CHAR_BLOCK_2D_ROW_SIZE) - 1;

    u32 tileY = verticalFlip ?
        (verticalOffset % 8) ^ 7 :
        verticalOffset % 8;

    bool colorMode = entry.attribute0.colorMode;
    u8 tileOffset = tileY * (colorMode ? CHAR_BLOCK_8_ROW_SIZE : CHAR_BLOCK_4_ROW_SIZE);
    u32 vramAddr = OBJ_CHAR_BLOCK_BASE_ADDR + (baseTileIndex * sizeof(CharBlockEntry4)) + tileOffset;
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
                u8 val = static_cast<u8>(vram[vramAddr + j]);
                colors[colorIndex++] = val & 0x0F;
                colors[colorIndex++] = val >> 4;
            }

            vramAddr += sizeof(CharBlockEntry4);
        }
    }
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

    if (doubleWidth && doubleHeight)
    {
        leftBaseBlockIndex += 2;
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
