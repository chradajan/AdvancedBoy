#pragma once

#include <string>
#include <vector>
#include <GBA/include/Types/Types.hpp>

namespace gui
{
enum class DisplayFormat
{
    BOOL,
    DEC,
    HEX
};

struct RegisterField
{
    std::string name;
    u32 mask;
    u8 shift;
    DisplayFormat format;
};

struct Register
{
    std::string name;
    std::string description;
    u32 addr;
    u8 size;
    std::vector<RegisterField> fields;
};

extern std::vector<Register> const IO_REGISTERS;
}  // namespace gui
