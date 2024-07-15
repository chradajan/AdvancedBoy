#pragma once

#include <GBA/include/Types.hpp>

namespace graphics
{
struct DISPCNT
{
    static constexpr size_t INDEX = 0;

    u16 bgMode                  : 3;
    u16 cgbMode                 : 1;
    u16 displayFrameSelect      : 1;
    u16 hBlankIntervalFree      : 1;
    u16 objCharacterVramMapping : 1;
    u16 forceBlank              : 1;
    u16 screenDisplayBg0        : 1;
    u16 screenDisplayBg1        : 1;
    u16 screenDisplayBg2        : 1;
    u16 screenDisplayBg3        : 1;
    u16 screenDisplayObj        : 1;
    u16 window0Display          : 1;
    u16 window1Display          : 1;
    u16 objWindowDisplay        : 1;
};

static_assert(sizeof(DISPCNT) == sizeof(u16), "DISPCNT must be 2 bytes");

struct DISPSTAT
{
    static constexpr size_t INDEX = 4;

    u16 vBlank              : 1;
    u16 hBlank              : 1;
    u16 vCounter            : 1;
    u16 vBlankIrqEnable     : 1;
    u16 hBlankIrqEnable     : 1;
    u16 vCounterIrqEnable   : 1;
    u16 unusedReadOnly      : 1;
    u16 unusedReadWrite     : 1;
    u16 vCountSetting       : 8;
};

static_assert(sizeof(DISPSTAT) == sizeof(u16), "DISPSTAT must be 2 bytes");

struct VCOUNT
{
    static constexpr size_t INDEX = 6;

    u16 ly  : 8;
    u16     : 8;
};

static_assert(sizeof(VCOUNT) == sizeof(u16), "VCOUNT must be 2 bytes");

struct BGCNT
{
    static constexpr size_t INDEX = 8;

    u16 priority        : 2;
    u16 charBaseBlock   : 2;
    u16                 : 2;
    u16 mosaic          : 1;
    u16 palette         : 1;
    u16 screenBaseBlock : 5;
    u16 wraparound      : 1;
    u16 screenSize      : 2;
};

static_assert(sizeof(BGCNT) == sizeof(u16), "BGCNT must be 2 bytes");
}  // namespace graphics
