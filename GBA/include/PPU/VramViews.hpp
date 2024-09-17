#pragma once

#include <cstddef>
#include <span>
#include <GBA/include/Utilities/Types.hpp>

namespace graphics { class PPU; }

namespace graphics
{
///---------------------------------------------------------------------------------------------------------------------------------
/// General VRAM types
///---------------------------------------------------------------------------------------------------------------------------------

constexpr u32 PRAM_SIZE = 1 * KiB;
constexpr u32 OAM_SIZE = 1 * KiB;
constexpr u32 VRAM_SIZE = 96 * KiB;

using VramSpan = std::span<const std::byte, VRAM_SIZE>;

constexpr u8 MAX_REG_SPRITE_WIDTH = 64;
using SpriteRow = std::array<u8, MAX_REG_SPRITE_WIDTH>;

///---------------------------------------------------------------------------------------------------------------------------------
/// OAM
///---------------------------------------------------------------------------------------------------------------------------------

/// @brief Representation of an entry in OAM.
struct OamEntry
{
    struct
    {
        u16 y           : 8;
        u16 objMode     : 2;
        u16 gfxMode     : 2;
        u16 mosaic      : 1;
        u16 colorMode   : 1;
        u16 shape       : 2;
    } attribute0;

    union
    {
        struct
        {
            u16 x       : 9;
            u16         : 5;
            u16 size    : 2;
        };

        struct
        {
            u16                 : 12;
            u16 horizontalFlip  : 1;
            u16 verticalFlip    : 1;
            u16                 : 2;
        };

        struct
        {
            u16                 : 9;
            u16 paramSelect     : 5;
            u16                 : 2;
        };
    } attribute1;

    struct
    {
        u16 tile        : 10;
        u16 priority    : 2;
        u16 palette     : 4;
    } attribute2;

    u16 padding;
};

static_assert(sizeof(OamEntry) == sizeof(u64), "OamEntry must be 8 bytes");

using Oam = std::span<const OamEntry, 128>;

/// @brief Representation of a single matrix for OBJ affine transformations.
struct AffineMatrix
{
    u16 pad0[3];
    i16 pa;

    u16 pad1[3];
    i16 pb;

    u16 pad2[3];
    i16 pc;

    u16 pad3[3];
    i16 pd;
};

static_assert(sizeof(AffineMatrix) == 4 * sizeof(OamEntry), "AffineMatrix must be 32 bytes");
static_assert(alignof(AffineMatrix) == alignof(OamEntry), "AffineMatrix and OamEntry must have same alignment");

constexpr u32 AFFINE_MATRIX_COUNT = OAM_SIZE / sizeof(AffineMatrix);
using AffineMatrices = std::span<const AffineMatrix, AFFINE_MATRIX_COUNT>;

///---------------------------------------------------------------------------------------------------------------------------------
/// Char blocks
///---------------------------------------------------------------------------------------------------------------------------------

/// @brief Representation of two color indexes in a 4bpp char block entry.
struct ColorIndexes4
{
    u8 leftColorIndex   : 4;
    u8 rightColorIndex  : 4;
};

static_assert(sizeof(ColorIndexes4) == sizeof(u8), "ColorIndexes4 must be 1 byte");

/// @brief Representation of a tile bitmap in 4bpp mode.
struct CharBlockEntry4
{
    ColorIndexes4 pixels[8][4];
};

static_assert(sizeof(CharBlockEntry4) == 32, "CharBlockEntry4 must be 32 bytes");

/// @brief Representation of a tile bitmap in 8bpp mode.
struct CharBlockEntry8
{
    u8 pixels[8][8];
};

static_assert(sizeof(CharBlockEntry8) == 64, "CharBlockEntry8 must be 64 bytes");

// General char block constants
constexpr u32 CHAR_BLOCK_SIZE = 16 * KiB;  // Size of a single char block in bytes.

constexpr u32 CHAR_BLOCK_4_ROW_SIZE = 4;  // Number of bytes in a row of a 4bpp char block entry.
constexpr u32 CHAR_BLOCK_8_ROW_SIZE = 8;  // Number of bytes in a row of an 8bpp char block entry.

// OBJ mapping constants
constexpr u32 OBJ_CHAR_BLOCKS_SIZE = 2 * CHAR_BLOCK_SIZE;  // Size of both OBJ char blocks in bytes.
constexpr u32 OBJ_CHAR_BLOCK_BASE_ADDR = 4 * CHAR_BLOCK_SIZE;  // Base address of the OBJ char blocks.
using ObjSpan = std::span<const std::byte, OBJ_CHAR_BLOCKS_SIZE>;

constexpr u32 OBJ_CHAR_BLOCK_TILE_COUNT = OBJ_CHAR_BLOCKS_SIZE / sizeof(CharBlockEntry4);  // Number of 4bpp sized OBJ tiles.

constexpr u32 CHAR_BLOCK_2D_TILES_PER_ROW = 32;  // Number of CharBlockEntry4 sized tiles in a row/column.
constexpr u32 CHAR_BLOCK_2D_ROW_SIZE = CHAR_BLOCK_2D_TILES_PER_ROW * sizeof(CharBlockEntry4);  // Size of a row of tiles in bytes.

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
    /// @param vram Span representing all of VRAM.
    /// @param baseIndex Base char block index in the range [0, 3].
    explicit BackgroundCharBlockView(VramSpan vram, u8 baseIndex);

    /// @brief Get the 4bpp char block entry at the specified index.
    /// @param block Reference to char block entry to set.
    /// @param index Tile index to get.
    void GetCharBlock(CharBlockEntry4& block, u16 index);

    /// @brief Get the 8bpp char block entry at the specified index.
    /// @param block Reference to char block entry to set.
    /// @param index Tile index to get.
    void GetCharBlock(CharBlockEntry8& block, u16 index);

    /// @brief Get the palette index from an 8bpp char block entry for an affine background.
    /// @param index Index of tile being accessed.
    /// @param tileX X-coordinate within the 8x8 char block entry.
    /// @param tileY Y-coordinate within the 8x8 char block entry.
    /// @return Palette index.
    u8 GetAffinePaletteIndex(u16 index, u8 tileX, u8 tileY);

private:
    std::span<const std::byte, BG_CHAR_BLOCKS_SIZE> charBlocks_;
    u32 const baseIndex_;
};

/// @brief Fetch the color index of each pixel in a single row for a regular sprite that uses 1D mapping.
/// @param vram Span representing the OBJ char blocks.
/// @param colors Array of color indexes to populate.
/// @param entry Reference to OAM entry to fetch data for.
/// @param width Width of the sprite in pixels.
/// @param height Height of the sprite in pixels.
/// @param verticalOffset Difference between current scanline and y-coordinate of the sprite.
void Populate1dRegularSpriteRow(ObjSpan vram, SpriteRow& colors, OamEntry const& entry, u8 width, u8 height, i16 verticalOffset);

/// @brief Fetch the color index of each pixel in a single row for a regular sprite that uses 2D mapping.
/// @param vram Span representing the OBJ char blocks.
/// @param colors Array of color indexes to populate.
/// @param entry Reference to OAM entry to fetch data for.
/// @param width Width of the sprite in pixels.
/// @param height Height of the sprite in pixels.
/// @param verticalOffset Difference between current scanline and y-coordinate of the sprite.
void Populate2dRegularSpriteRow(ObjSpan vram, SpriteRow& colors, OamEntry const& entry, u8 width, u8 height, i16 verticalOffset);

/// @brief Get the color index of a pixel within an OBJ char block using 1D mapping.
/// @param vram Span representing the OBJ char blocks.
/// @param baseTile Base tile of the sprite.
/// @param textureX X-coordinate within the sprite.
/// @param textureY Y-coordinate within the sprite.
/// @param widthTiles Width of the sprite in tiles (same regardless of 4bpp vs 8bpp).
/// @param colorMode True for 8bpp, false for 4bpp.
/// @return Color index within a sprite.
u8 Get1dAffineColorIndex(ObjSpan vram, u16 baseTile, i32 textureX, i32 textureY, u8 widthTiles, bool colorMode);

/// @brief Get the color index of a pixel within an OBJ char block using 2D mapping.
/// @param vram Span representing the OBJ char blocks.
/// @param baseTile Base tile of the sprite.
/// @param textureX X-coordinate within the sprite.
/// @param textureY Y-coordinate within the sprite.
/// @param colorMode True for 8bpp, false for 4bpp.
/// @return Color index within a sprite.
u8 Get2dAffineColorIndex(ObjSpan vram, u16 baseTile, i32 textureX, i32 textureY, bool colorMode);

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
