#include <GBA/include/PPU/FrameBuffer.hpp>
#include <algorithm>
#include <array>
#include <vector>
#include <GBA/include/PPU/Registers.hpp>
#include <GBA/include/Types/Types.hpp>

namespace
{
/// @brief Blend the top two layers together.
/// @param eva First target coefficient.
/// @param evb Second target coefficient.
/// @param targetA bgr555 value of first target.
/// @param targetB bgr555 value of second target.
/// @return Blended bgr555 value of the two pixels.
inline u16 AlphaBlend(u16 eva, u16 evb, u16 targetA, u16 targetB)
{
    // Isolate individual r, g, and b intensities as 1.4 fixed point values
    u16 redA = (targetA & 0x001F) << 4;
    u16 redB = (targetB & 0x001F) << 4;
    u16 greenA = (targetA & 0x03E0) >> 1;
    u16 greenB = (targetB & 0x03E0) >> 1;
    u16 blueA = (targetA & 0x7C00) >> 6;
    u16 blueB = (targetB & 0x7C00) >> 6;

    u16 red = ((eva * redA) + (evb * redB)) >> 8;
    u16 green = ((eva * greenA) + (evb * greenB)) >> 8;
    u16 blue = ((eva * blueA) + (evb * blueB)) >> 8;

    red = std::min(static_cast<u16>(31), red);
    green = std::min(static_cast<u16>(31), green);
    blue = std::min(static_cast<u16>(31), blue);

    return (blue << 10) | (green << 5) | red;
}

/// @brief Increase the brightness of the top layer.
/// @param evy Brightness coefficient.
/// @param target bgr555 value to increase brightness of.
/// @return Increased brightness bgr555 value.
inline u16 IncreaseBrightness(u16 evy, u16 target)
{
    // Isolate individual r, g, and b intensities as 1.4 fixed point values
    u16 red = (target & 0x001F) << 4;
    u16 green = (target & 0x03E0) >> 1;
    u16 blue = (target & 0x7C00) >> 6;

    red = (red + (((0x01F0 - red) * evy) >> 4)) >> 4;
    green = (green + (((0x01F0 - green) * evy) >> 4)) >> 4;
    blue = (blue + (((0x01F0 - blue) * evy) >> 4)) >> 4;

    return (blue << 10) | (green << 5) | red;
}

/// @brief Decrease the brightness of the top layer.
/// @param evy Brightness coefficient.
/// @param target bgr555 value to decrease brightness of.
/// @return Decreased brightness bgr555 value.
inline u16 DecreaseBrightness(u16 evy, u16 target)
{
    // Isolate individual r, g, and b intensities as 1.4 fixed point values
    u16 red = (target & 0x001F) << 4;
    u16 green = (target & 0x03E0) >> 1;
    u16 blue = (target & 0x7C00) >> 6;

    red = (red - ((red * evy) >> 4)) >> 4;
    green = (green - ((green * evy) >> 4)) >> 4;
    blue = (blue - ((blue * evy) >> 4)) >> 4;

    return (blue << 10) | (green << 5) | red;
}
}

namespace graphics
{
bool Pixel::operator<(Pixel const& rhs) const
{
    if (transparent && !rhs.transparent)
    {
        return false;
    }

    if (!transparent && rhs.transparent)
    {
        return true;
    }

    if (priority == rhs.priority)
    {
        return source < rhs.source;
    }

    return priority < rhs.priority;
}

FrameBuffer::FrameBuffer()
{
    for (auto& pixels : scanline_)
    {
        pixels.reserve(5);
    }

    for (auto& frameBuffer : frameBuffers_)
    {
        frameBuffer.fill(0xFFFF);
    }

    activeBufferIndex_ = 0;
    pixelIndex_ = 0;
}

void FrameBuffer::PushPixel(Pixel pixel, u8 dot)
{
    scanline_.at(dot).push_back(pixel);
}

void FrameBuffer::PushSpritePixels()
{
    for (u8 dot = 0; dot < LCD_WIDTH; ++dot)
    {
        if (spriteScanline_[dot].initialized)
        {
            scanline_[dot].push_back(spriteScanline_[dot]);
        }
    }
}

void FrameBuffer::ClearSpritePixels()
{
    for (Pixel& pixel : spriteScanline_)
    {
        pixel.initialized = false;
    }
}

void FrameBuffer::RenderScanline(u16 backdrop, bool forceBlank, BLDCNT bldcnt, BLDALPHA bldalpha, BLDY bldy)
{
    if (forceBlank)
    {
        for (auto& pixels : scanline_)
        {
            pixels.clear();
            frameBuffers_[activeBufferIndex_].at(pixelIndex_++) = 0x7FFF;
        }

        return;
    }

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wnarrowing"
    std::array<bool, 6> firstTargetLayer = {bldcnt.objA, bldcnt.bg0A, bldcnt.bg1A, bldcnt.bg2A, bldcnt.bg3A, bldcnt.bdA};
    std::array<bool, 6> secondTargetLayer = {bldcnt.objB, bldcnt.bg0B, bldcnt.bg1B, bldcnt.bg2B, bldcnt.bg3B, bldcnt.bdB};
    #pragma GCC diagnostic pop

    Pixel bdPixel = Pixel(PixelSrc::BD, backdrop, 4, false);
    Pixel* pixelA = nullptr;
    Pixel* pixelB = nullptr;

    SpecialEffect bldcntEffect = static_cast<SpecialEffect>(bldcnt.specialEffect);

    // Coefficients are 1.4 fixed point values
    u16 eva = std::min(bldalpha.evaCoefficient, static_cast<u16>(0x10));
    u16 evb = std::min(bldalpha.evbCoefficient, static_cast<u16>(0x10));
    u16 evy = std::min(bldy.evyCoefficient, static_cast<u16>(0x10));

    for (u8 dot = 0; dot < LCD_WIDTH; ++dot)
    {
        auto& pixels = scanline_[dot];

        switch (pixels.size())
        {
            case 0:
                pixelA = nullptr;
                pixelB = nullptr;
                break;
            case 1:
                pixelA = &pixels[0];
                pixelB = nullptr;
                break;
            default:
                std::nth_element(pixels.begin(), pixels.begin() + 1, pixels.end());
                pixelA = &pixels[0];
                pixelB = &pixels[1];
                break;
        }

        if (!pixelA || pixelA->transparent)
        {
            pixelA = &bdPixel;
            pixelB = nullptr;
        }
        else if (pixelB && pixelB->transparent)
        {
            pixelB = nullptr;
        }

        u16 bgr555 = pixelA->color;
        SpecialEffect actualEffect = bldcntEffect;

        if ((pixelA->semiTransparent) && pixelB && !pixelB->transparent)
        {
            actualEffect = SpecialEffect::AlphaBlending;
        }
        else if (!windowScanline_[dot].effectsEnabled)
        {
            actualEffect = SpecialEffect::None;
        }

        switch (actualEffect)
        {
            case SpecialEffect::None:
                break;
            case SpecialEffect::AlphaBlending:
            {
                if (pixelB && (firstTargetLayer[static_cast<u8>(pixelA->source)] || pixelA->semiTransparent) && secondTargetLayer[static_cast<u8>(pixelB->source)])
                {
                    bgr555 = AlphaBlend(eva, evb, pixelA->color, pixelB->color);
                }

                break;
            }
            case SpecialEffect::BrightnessIncrease:
            {
                if (firstTargetLayer[static_cast<u8>(pixelA->source)])
                {
                    bgr555 = IncreaseBrightness(evy, pixelA->color);
                }

                break;
            }
            case SpecialEffect::BrightnessDecrease:
            {
                if (firstTargetLayer[static_cast<u8>(pixelA->source)])
                {
                    bgr555 = DecreaseBrightness(evy, pixelA->color);
                }

                break;
            }
        }

        frameBuffers_[activeBufferIndex_].at(pixelIndex_++) = bgr555;
        pixels.clear();
    }
}

void FrameBuffer::ResetFrameIndex()
{
    activeBufferIndex_ = (activeBufferIndex_ + 1) % frameBuffers_.size();
    pixelIndex_ = 0;
}

uchar* FrameBuffer::GetRawFrameBuffer()
{
    u8 index = (activeBufferIndex_ == 0) ? frameBuffers_.size() - 1 : activeBufferIndex_ - 1;
    return reinterpret_cast<uchar*>(frameBuffers_.at(index).data());
}
}  // namespace graphics
