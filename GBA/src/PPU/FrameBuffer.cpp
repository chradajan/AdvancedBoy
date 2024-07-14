#include <GBA/include/PPU/FrameBuffer.hpp>

namespace graphics
{
FrameBuffer::FrameBuffer()
{
    for (auto& frameBuffer : frameBuffers_)
    {
        frameBuffer.fill(0xFFFF);
    }

    activeBufferIndex_ = 0;
}

u8* FrameBuffer::GetRawFrameBuffer()
{
    u8 index = (activeBufferIndex_ == 0) ? frameBuffers_.size() - 1 : activeBufferIndex_ - 1;
    return reinterpret_cast<u8*>(frameBuffers_.at(index).data());
}
}  // namespace graphics
