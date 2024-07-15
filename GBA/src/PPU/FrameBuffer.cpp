#include <GBA/include/PPU/FrameBuffer.hpp>

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

void FrameBuffer::PushPixel(Pixel pixel, int dot)
{
    scanline_.at(dot).push_back(pixel);
}

void FrameBuffer::RenderScanline(u16 backdrop, bool forceBlank)
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

    for (auto& pixels : scanline_)
    {
        u16 color = pixels.empty() ? backdrop : pixels.front().color;
        frameBuffers_[activeBufferIndex_].at(pixelIndex_++) = color;
        pixels.clear();
    }
}

void FrameBuffer::ResetFrameIndex()
{
    activeBufferIndex_ = (activeBufferIndex_ + 1) % frameBuffers_.size();
    pixelIndex_ = 0;
}

u8* FrameBuffer::GetRawFrameBuffer()
{
    u8 index = (activeBufferIndex_ == 0) ? frameBuffers_.size() - 1 : activeBufferIndex_ - 1;
    return reinterpret_cast<u8*>(frameBuffers_.at(index).data());
}
}  // namespace graphics
