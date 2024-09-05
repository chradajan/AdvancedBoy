#pragma once

#include <GBA/include/Types/Types.hpp>

struct KEYINPUT
{
    static constexpr size_t STATUS_INDEX = 0x00;
    static constexpr size_t CONTROL_INDEX = 0x02;
    static constexpr u16 BUTTON_MASK = 0x03FF;
    static constexpr u16 DEFAULT_KEYPAD_STATE = 0x03FF;

    u16 A      : 1;
    u16 B      : 1;
    u16 Select : 1;
    u16 Start  : 1;
    u16 Right  : 1;
    u16 Left   : 1;
    u16 Up     : 1;
    u16 Down   : 1;
    u16 R      : 1;
    u16 L      : 1;
    u16        : 4;
    u16 IRQ    : 1;
    u16 Cond   : 1;
};

static_assert(sizeof(KEYINPUT) == sizeof(u16), "KEYINPUT must be 2 bytes");
