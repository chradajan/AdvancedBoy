#pragma once

#include <GBA/include/Types/Types.hpp>

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

/// @brief Get the 16MiB memory page that from an address.
/// @param addr Address to find the page it resides in.
/// @return Page that address resides in.
inline Page GetMemPage(u32 addr)
{
    if (addr >= 0x1000'0000)
    {
        return Page::INVALID;
    }

    u8 page = (addr & 0x0F00'0000) >> 24;
    return Page{page};
}

// Memory page bounds

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

// IO region bounds

constexpr u32 LCD_IO_ADDR_MIN = 0x0400'0000;
constexpr u32 LCD_IO_ADDR_MAX = 0x0400'0057;

constexpr u32 SOUND_IO_ADDR_MIN = 0x0400'0060;
constexpr u32 SOUND_IO_ADDR_MAX = 0x0400'00A7;

constexpr u32 DMA_IO_ADDR_MIN = 0x0400'00B0;
constexpr u32 DMA_IO_ADDR_MAX = 0x0400'00DF;

constexpr u32 TIMER_IO_ADDR_MIN = 0x0400'0100;
constexpr u32 TIMER_IO_ADDR_MAX = 0x0400'010F;

constexpr u32 SERIAL_IO_1_ADDR_MIN = 0x0400'0120;
constexpr u32 SERIAL_IO_1_ADDR_MAX = 0x0400'012B;

constexpr u32 KEYPAD_IO_ADDR_MIN = 0x0400'0130;
constexpr u32 KEYPAD_IO_ADDR_MAX = 0x0400'0133;

constexpr u32 SERIAL_IO_2_ADDR_MIN = 0x0400'0134;
constexpr u32 SERIAL_IO_2_ADDR_MAX = 0x0400'015B;

constexpr u32 SYSTEM_CONTROL_IO_ADDR_MIN = 0x0400'0200;
constexpr u32 SYSTEM_CONTROL_IO_ADDR_MAX = 0x0400'0803;

// APU registers

constexpr u32 CHANNEL_1_ADDR_MIN = 0x0400'0060;
constexpr u32 CHANNEL_1_ADDR_MAX = 0x0400'0067;

constexpr u32 CHANNEL_2_ADDR_MIN = 0x0400'0068;
constexpr u32 CHANNEL_2_ADDR_MAX = 0x0400'006F;

constexpr u32 CHANNEL_3_ADDR_MIN = 0x0400'0070;
constexpr u32 CHANNEL_3_ADDR_MAX = 0x0400'0077;

constexpr u32 CHANNEL_4_ADDR_MIN = 0x0400'0078;
constexpr u32 CHANNEL_4_ADDR_MAX = 0x0400'007F;

constexpr u32 APU_CONTROL_ADDR_MIN = 0x0400'0080;
constexpr u32 APU_CONTROL_ADDR_MAX = 0x0400'008B;

constexpr u32 WAVE_RAM_ADDR_MIN = 0x0400'0090;
constexpr u32 WAVE_RAM_ADDR_MAX = 0x0400'009F;

constexpr u32 DMA_AUDIO_ADDR_MIN = 0x0400'00A0;
constexpr u32 DMA_AUDIO_ADDR_MAX = 0x0400'00A7;

constexpr u32 FIFO_A_ADDR = 0x0400'00A0;
constexpr u32 FIFO_B_ADDR = 0x0400'00A4;

// DMA registers

constexpr u32 DMA0_ADDR_MIN = 0x0400'00B0;
constexpr u32 DMA0_ADDR_MAX = 0x0400'00BB;

constexpr u32 DMA1_ADDR_MIN = 0x0400'00BC;
constexpr u32 DMA1_ADDR_MAX = 0x0400'00C7;

constexpr u32 DMA2_ADDR_MIN = 0x0400'00C8;
constexpr u32 DMA2_ADDR_MAX = 0x0400'00D3;

constexpr u32 DMA3_ADDR_MIN = 0x0400'00D4;
constexpr u32 DMA3_ADDR_MAX = 0x0400'00DF;

// GamePak Regions

constexpr u32 EEPROM_SMALL_CART_ADDR_MIN = 0x0D00'0000;
constexpr u32 EEPROM_LARGE_CART_ADDR_MIN = 0x0DFF'FF00;
constexpr u32 EEPROM_ADDR_MAX = 0x0DFF'FFFF;

constexpr u32 SRAM_ADDR_MIN = 0x0E00'0000;
constexpr u32 SRAM_ADDR_MAX = 0x0FFF'FFFF;

constexpr u32 FLASH_ADDR_MIN = 0x0E00'0000;
constexpr u32 FLASH_ADDR_MAX = 0x0E00'FFFF;

// System control regions

constexpr u32 INT_WAITCNT_ADDR_MIN = 0x0400'0200;
constexpr u32 INT_WAITCNT_ADDR_MAX = 0x0400'020B;

constexpr u32 POSTFLG_HALTCNT_ADDR_MIN = 0x0400'0300;
constexpr u32 POSTFLG_HALTCNT_ADDR_MAX = 0x0400'0303;

constexpr u32 INTERNAL_MEM_CONTROL_ADDR_MIN = 0x0400'0800;
constexpr u32 INTERNAL_MEM_CONTROL_ADDR_MAX = 0x0400'0803;

// Timer registers

constexpr u32 TIMER_0_ADDR_MIN = 0x0400'0100;
constexpr u32 TIMER_0_ADDR_MAX = 0x0400'0103;

constexpr u32 TIMER_1_ADDR_MIN = 0x0400'0104;
constexpr u32 TIMER_1_ADDR_MAX = 0x0400'0107;

constexpr u32 TIMER_2_ADDR_MIN = 0x0400'0108;
constexpr u32 TIMER_2_ADDR_MAX = 0x0400'010B;

constexpr u32 TIMER_3_ADDR_MIN = 0x0400'010C;
constexpr u32 TIMER_3_ADDR_MAX = 0x0400'010F;
