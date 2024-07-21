#pragma once

#include <array>
#include <vector>
#include <GBA/include/PPU/Registers.hpp>
#include <GBA/include/Types.hpp>

namespace graphics
{
constexpr size_t LCD_WIDTH = 240;
constexpr size_t LCD_HEIGHT = 160;

enum class PixelSrc : u8
{
    OBJ = 0,
    BG0,
    BG1,
    BG2,
    BG3,
    BD
};

enum class SpecialEffect : u8
{
    None = 0,
    AlphaBlending,
    BrightnessIncrease,
    BrightnessDecrease
};

struct WindowSettings
{
    std::array<bool, 4> bgEnabled;
    bool objEnabled;
    bool effectsEnabled;
};

class Pixel
{
public:
    /// @brief Set default constructed pixels to be uninitialized.
    Pixel() : initialized(false) {}

    /// @brief Create a pixel.
    /// @param src Source that generated this pixel.
    /// @param bgr555 Color value of this pixel.
    /// @param prio Priority (0-3) of this pixel.
    /// @param transparency Whether this pixel is transparent.
    /// @param semiTransparency If this is an OBJ pixel, then whether this pixel is part of a semi-transparent sprite.
    Pixel(PixelSrc src, u16 bgr555, u8 prio, bool transparency, bool semiTransparency = false) :
        source(src),
        color(bgr555),
        priority(prio),
        transparent(transparency),
        semiTransparent(semiTransparency)
    {
    }

    /// @brief Compare priority of two pixels. If A < B, then A has a higher priority and should be chosen to be drawn.
    /// @param rhs Pixel to compare to.
    /// @return Relative priority of two pixels.
    bool operator<(Pixel const& rhs) const;

    PixelSrc source;
    u16 color;
    u8 priority;
    bool transparent;
    bool semiTransparent;
    bool initialized;
};

class FrameBuffer
{
    using PixelBuffer = std::array<u16, LCD_WIDTH * LCD_HEIGHT>;

public:
    FrameBuffer(FrameBuffer const&) = delete;
    FrameBuffer& operator=(FrameBuffer const&) = delete;
    FrameBuffer(FrameBuffer&&) = delete;
    FrameBuffer& operator=(FrameBuffer&&) = delete;

    /// @brief Initialize empty frame buffers.
    FrameBuffer();

    /// @brief Add a pixel to be considered for drawing at the specified dot on the current scanline.
    /// @param pixel Pixel to potentially be drawn.
    /// @param dot Dot on the current scanline to add pixel at.
    void PushPixel(Pixel pixel, u8 dot);

    /// @brief Get a sprite pixel on the current scanline at a particular dot.
    /// @param dot Dot to get sprite pixel at.
    /// @return Reference to pixel at specified dot.
    Pixel& GetSpritePixel(size_t dot) { return spriteScanline_.at(dot); }

    /// @brief Merge the sprite pixels scanline buffer into the main scanline buffer.
    void PushSpritePixels();

    /// @brief Uninitialize all pixels in sprite scanline buffer.
    void ClearSpritePixels();

    /// @brief Render the current scanline of pixels to the frame buffer.
    /// @param backdrop Pixel color of backdrop layer.
    /// @param forceBlank If true, make each pixel in the current scanline white.
    /// @param bldcnt BLDCNT register value.
    /// @param bldalpha BLDALPHA register value.
    /// @param bldy BLDY register value.
    void RenderScanline(u16 backdrop, bool forceBlank, BLDCNT bldcnt, BLDALPHA bldalpha, BLDY bldy);

    /// @brief Call on VBlank. Mark the current frame as complete and prepare to render to the next one.
    void ResetFrameIndex();

    /// @brief Initialize the window settings by setting each pixel to the specified default setting.
    /// @param defaultSettings Settings to apply to each pixel.
    void InitializeWindow(WindowSettings defaultSettings) { windowScanline_.fill(defaultSettings); }

    /// @brief Get the window settings at the specified pixel.
    /// @param dot Index of window settings to get.
    /// @return Reference to window settings.
    WindowSettings& GetWindowSettings(u8 dot) { return windowScanline_.at(dot); }

    /// @brief Get a pointer to the pixel data of the most recently completed frame.
    /// @return Pointer to raw pixel data.
    uchar* GetRawFrameBuffer();

private:
    // Current scanline data
    std::array<std::vector<Pixel>, LCD_WIDTH> scanline_;
    std::array<Pixel, LCD_WIDTH> spriteScanline_;
    std::array<WindowSettings, LCD_WIDTH> windowScanline_;

    // Raw pixel data
    std::array<PixelBuffer, 3> frameBuffers_;
    size_t pixelIndex_;
    u8 activeBufferIndex_;
};
}  // namespace graphics
