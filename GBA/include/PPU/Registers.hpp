#pragma once

#include <GBA/include/Types/Types.hpp>

namespace graphics
{
struct DISPCNT
{
    static constexpr size_t INDEX = 0x00;

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
    static constexpr size_t INDEX = 0x04;

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
    static constexpr size_t INDEX = 0x06;

    u16 ly  : 8;
    u16     : 8;
};

static_assert(sizeof(VCOUNT) == sizeof(u16), "VCOUNT must be 2 bytes");

struct BGCNT
{
    static constexpr size_t INDEX = 0x08;

    u16 priority        : 2;
    u16 charBaseBlock   : 2;
    u16                 : 2;
    u16 mosaic          : 1;
    u16 palette         : 1;
    u16 screenBaseBlock : 5;
    u16 wrapAround      : 1;
    u16 screenSize      : 2;
};

static_assert(sizeof(BGCNT) == sizeof(u16), "BGCNT must be 2 bytes");

struct WININ
{
    static constexpr size_t INDEX = 0x48;

    u16 win0Bg0Enabled      : 1;
    u16 win0Bg1Enabled      : 1;
    u16 win0Bg2Enabled      : 1;
    u16 win0Bg3Enabled      : 1;
    u16 win0ObjEnabled      : 1;
    u16 win0SpecialEffect   : 1;
    u16                     : 2;
    u16 win1Bg0Enabled      : 1;
    u16 win1Bg1Enabled      : 1;
    u16 win1Bg2Enabled      : 1;
    u16 win1Bg3Enabled      : 1;
    u16 win1ObjEnabled      : 1;
    u16 win1SpecialEffect   : 1;
    u16                     : 2;
};

static_assert(sizeof(WININ) == sizeof(u16), "WININ must be 2 bytes");

struct WINOUT
{
    static constexpr size_t INDEX = 0x4A;

    u16 outsideBg0Enabled       : 1;
    u16 outsideBg1Enabled       : 1;
    u16 outsideBg2Enabled       : 1;
    u16 outsideBg3Enabled       : 1;
    u16 outsideObjEnabled       : 1;
    u16 outsideSpecialEffect    : 1;
    u16                         : 2;
    u16 objWinBg0Enabled        : 1;
    u16 objWinBg1Enabled        : 1;
    u16 objWinBg2Enabled        : 1;
    u16 objWinBg3Enabled        : 1;
    u16 objWinObjEnabled        : 1;
    u16 objWinSpecialEffect     : 1;
    u16                         : 2;
};

static_assert(sizeof(WINOUT) == sizeof(u16), "WINOUT must be 2 bytes");

struct BLDCNT
{
    static constexpr size_t INDEX = 0x50;

    u16 bg0A            : 1;
    u16 bg1A            : 1;
    u16 bg2A            : 1;
    u16 bg3A            : 1;
    u16 objA            : 1;
    u16 bdA             : 1;
    u16 specialEffect   : 2;
    u16 bg0B            : 1;
    u16 bg1B            : 1;
    u16 bg2B            : 1;
    u16 bg3B            : 1;
    u16 objB            : 1;
    u16 bdB             : 1;
    u16                 : 2;
};

static_assert(sizeof(BLDCNT) == sizeof(u16), "BLDCNT must be 2 bytes");

struct BLDALPHA
{
    static constexpr size_t INDEX = 0x52;

    u16 evaCoefficient  : 5;
    u16                 : 3;
    u16 evbCoefficient  : 5;
    u16                 : 3;
};

static_assert(sizeof(BLDALPHA) == sizeof(u16), "BLDALPHA must be 2 bytes");

struct BLDY
{
    static constexpr size_t INDEX = 0x54;

    u16 evyCoefficient  : 5;
    u16                 : 11;
};

static_assert(sizeof(BLDY) == sizeof(u16), "BLDY must be 2 bytes");
}  // namespace graphics
