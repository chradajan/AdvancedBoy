#pragma once

#include <cstddef>
#include <span>
#include <GBA/include/Types.hpp>

namespace graphics { class PPU; }

namespace graphics
{
///---------------------------------------------------------------------------------------------------------------------------------
/// Char blocks
///---------------------------------------------------------------------------------------------------------------------------------

static constexpr u32 CHAR_BLOCK_SIZE = 16 * KiB;

/// @brief Representation of a tile bitmap in 4bpp mode.
struct CharBlockEntry4
{
    struct ColorIndexes
    {
        u8 leftNibble   : 4;
        u8 rightNibble  : 4;
    };

    ColorIndexes pixels[8][4];
};

static_assert(sizeof(CharBlockEntry4) == 32, "CharBlockEntry4 must be 32 bytes");

/// @brief Representation of a tile bitmap in 8bpp mode.
struct CharBlockEntry8
{
    u8 pixels[8][8];
};

static_assert(sizeof(CharBlockEntry8) == 64, "CharBlockEntry8 must be 64 bytes");

/// @brief View of the background char blocks. Does not alter data in VRAM.
class BackgroundCharBlockView
{
    static constexpr u32 BG_CHAR_BLOCK_COUNT = 4;
    static constexpr u32 BG_CHAR_BLOCKS_SIZE = CHAR_BLOCK_SIZE * BG_CHAR_BLOCK_COUNT;

public:
    BackgroundCharBlockView() = delete;
    BackgroundCharBlockView(BackgroundCharBlockView const&) = delete;
    BackgroundCharBlockView& operator=(BackgroundCharBlockView const&) = delete;
    BackgroundCharBlockView(BackgroundCharBlockView&&) = delete;
    BackgroundCharBlockView& operator=(BackgroundCharBlockView&&) = delete;

    /// @brief Initialize a char block view for the background char blocks.
    /// @param ppu Reference to PPU needed for VRAM access.
    /// @param baseIndex Base char block index in the range [0, 3].
    explicit BackgroundCharBlockView(PPU const& ppu, u8 baseIndex);

    /// @brief Get the 4bpp char block entry at the specified index.
    /// @param block Reference to char block entry to set.
    /// @param index Tile index to get.
    void GetCharBlock(CharBlockEntry4& block, u16 index);

    /// @brief Get the 8bpp char block entry at the specified index.
    /// @param block Reference to char block entry to set.
    /// @param index Tile index to get.
    void GetCharBlock(CharBlockEntry8& block, u16 index);

private:
    std::span<const std::byte, BG_CHAR_BLOCKS_SIZE> charBlocks_;
    u32 const baseIndex_;
};

///---------------------------------------------------------------------------------------------------------------------------------
/// Screen blocks
///---------------------------------------------------------------------------------------------------------------------------------

constexpr u32 SCREEN_BLOCK_SIZE = 2 * KiB;
using ScreenBlock = std::span<const std::byte, SCREEN_BLOCK_SIZE>;

/// @brief Representation of an entry in the screen block map.
struct ScreenBlockEntry
{
    u16 tile            : 10;
    u16 horizontalFlip  : 1;
    u16 verticalFlip    : 1;
    u16 palette         : 4;
};

static_assert(sizeof(ScreenBlockEntry) == sizeof(u16), "ScreenBlockEntry must be 2 bytes");

/// @brief View of a single row of screen block entries in the tile map of a regular tiled background. Does not alter data in VRAM.
class RegularScreenBlockScanlineView
{
public:
    RegularScreenBlockScanlineView() = delete;
    RegularScreenBlockScanlineView(RegularScreenBlockScanlineView const&) = delete;
    RegularScreenBlockScanlineView& operator=(RegularScreenBlockScanlineView const&) = delete;
    RegularScreenBlockScanlineView(RegularScreenBlockScanlineView&&) = delete;
    RegularScreenBlockScanlineView& operator=(RegularScreenBlockScanlineView&&) = delete;

    /// @brief Initialize a screen block view. Also grabs the screen block entry data at the starting location.
    /// @param ppu Reference to PPU needed for VRAM access.
    /// @param baseIndex Index of base screen block.
    /// @param x Starting x-coordinate within the screen map in terms of pixels.
    /// @param y Starting y-coordinate within the screen map in terms of pixels.
    /// @param width Width in pixels of the screen map. Must be either 256 or 512.
    explicit RegularScreenBlockScanlineView(PPU const& ppu, u8 baseIndex, u16 x, u16 y, u16 width);

    /// @brief Increment the pixel position within the map. Updates the screen block entry data if the increment advances to the
    ///        the next entry.
    /// @return Whether this update advanced the view to the next entry. If true, any references to the previous entry's palette or
    ///         tile index should be updated.
    bool Update();

    /// @brief Get the x-coordinate within an 8x8 pixel tile bitmap based on this view's current location.
    /// @return X-coordinate within the corresponding tile bitmap in the range of [0, 7]. Updated after each call to Update.
    u8 TileX() const { return tileX_; }

    /// @brief Get the y-coordinate within an 8x8 pixel tile bitmap based on this view's current location.
    /// @return Y-coordinate within the corresponding tile bitmap in the range of [0, 7]. Updated after each call to Update.
    u8 TileY() const { return tileY_; }

    /// @brief Get the palette index [0, 15] corresponding to the view's current location.
    /// @return Palette index. Updated whenever Update advances to the next screen block entry.
    u8 Palette() const { return palette_; }

    /// @brief Get the tile index [0, 1023] corresponding to the view's current location.
    /// @return Tile index. Updated whenever Update advances to the next screen block entry.
    u16 TileIndex() const { return tile_; }

private:
    static constexpr u32 SCREEN_BLOCK_WIDTH = 32;
    static constexpr u32 SCREEN_BLOCK_SCANLINE_SIZE = SCREEN_BLOCK_WIDTH * sizeof(ScreenBlockEntry);

    // Left and right screen block scanline views
    std::span<const std::byte> left_;
    std::span<const std::byte> right_;

    // Current location info
    u16 x_;
    u16 const y_;

    u8 screenBlockX_;
    u16 const width_;

    // Tile coordinate
    u8 tileX_;
    u8 tileY_;

    // Current screen block entry data
    bool horizontalFlip_;
    bool verticalFlip_;
    u8 palette_;
    u16 tile_;
};
}
