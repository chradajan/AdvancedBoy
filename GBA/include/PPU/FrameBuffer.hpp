#pragma once

#include <array>
#include <GBA/include/Types.hpp>

namespace graphics
{
constexpr size_t LCD_WIDTH = 240;
constexpr size_t LCD_HEIGHT = 160;

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

    /// @brief Get a pointer to the pixel data of the most recently completed frame.
    /// @return Pointer to raw pixel data.
    u8* GetRawFrameBuffer();

private:
    std::array<PixelBuffer, 3> frameBuffers_;
    u8 activeBufferIndex_;
};
}  // namespace graphics
