#pragma once

#include <GBA/include/Types.hpp>

enum class Page : u8
{
    BIOS        = 0x00,
    EWRAM       = 0x02,
    IWRAM       = 0x03,
    IO          = 0x04,
    PRAM        = 0x05,
    VRAM        = 0x06,
    OAM         = 0x07,
    GAMEPAK_MIN = 0x08,
    GAMEPAK_MAX = 0x0F,
    INVALID     = 0xFF,
};
