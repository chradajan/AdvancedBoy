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
