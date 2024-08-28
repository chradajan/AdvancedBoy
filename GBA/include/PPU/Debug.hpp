#pragma once

#include <vector>
#include <GBA/include/Types.hpp>

namespace graphics
{
struct BackgroundDebugInfo
{
    // Image
    std::vector<u16> buffer;
    u16 width;
    u16 height;

    // Info
    u8 priority;
    u32 mapBaseAddr;
    u32 tileBaseAddr;

    // Regular
    bool regular;
    u16 xOffset;
    u16 yOffset;

    // Affine
    float refX;
    float refY;
    float pa;
    float pb;
    float pc;
    float pd;
};
}  // namespace graphics
