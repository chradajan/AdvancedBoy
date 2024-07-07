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

constexpr u32 BIOS_ADDR_MIN = 0x0000'0000;
constexpr u32 BIOS_ADDR_MAX = 0x0000'3FFF;

constexpr u32 EWRAM_ADDR_MIN = 0x0200'0000;
constexpr u32 EWRAM_ADDR_MAX = 0x0203'FFFF;

constexpr u32 IWRAM_ADDR_MIN = 0x0300'0000;
constexpr u32 IWRAM_ADDR_MAX = 0x0300'7FFF;

constexpr u32 IO_ADDR_MIN = 0x0400'0000;
constexpr u32 IO_ADDR_MAX = 0x0400'03FE;

constexpr u32 PRAM_ADDR_MIN = 0x0500'0000;
constexpr u32 PRAM_ADDR_MAX = 0x0500'03FF;

constexpr u32 VRAM_ADDR_MIN = 0x0600'0000;
constexpr u32 VRAM_ADDR_MAX = 0x0601'7FFF;

constexpr u32 OAM_ADDR_MIN = 0x0700'0000;
constexpr u32 OAM_ADDR_MAX = 0x0700'03FF;

constexpr u32 GAMEPAK_ROM_ADDR_MIN = 0x0800'0000;
constexpr u32 GAMEPAK_ROM_ADDR_MAX = 0x0DFF'FFFF;

constexpr u32 GAMEPAK_SRAM_ADDR_MIN = 0x0E00'0000;
constexpr u32 GAMEPAK_SRAM_ADDR_MAX = 0x0E00'FFFF;
