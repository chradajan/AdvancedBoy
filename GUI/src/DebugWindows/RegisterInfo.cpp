#include <GUI/include/DebugWindows/RegisterInfo.hpp>
#include <string>
#include <vector>
#include <GBA/include/Utilities/Types.hpp>

namespace gui
{
std::vector<Register> const IO_REGISTERS = {
    ///-----------------------------------------------------------------------------------------------------------------------------
    /// PPU
    ///-----------------------------------------------------------------------------------------------------------------------------

    // DISPCNT
    {
        "DISPCNT",
        "LCD Control",
        0x0400'0000,
        2,
        {
            {"BG Mode",                 0x0007,         0,      DisplayFormat::DEC},
            {"CGB Mode",                0x0008,         3,      DisplayFormat::BOOL},
            {"Frame Select",            0x0010,         4,      DisplayFormat::BOOL},
            {"H-Blank Interval Free",   0x0020,         5,      DisplayFormat::BOOL},
            {"Linear OBJ Mapping",      0x0040,         6,      DisplayFormat::BOOL},
            {"Force Blank",             0x0080,         7,      DisplayFormat::BOOL},
            {"Enable BG0",              0x0100,         8,      DisplayFormat::BOOL},
            {"Enable BG1",              0x0200,         9,      DisplayFormat::BOOL},
            {"Enable BG2",              0x0400,         10,     DisplayFormat::BOOL},
            {"Enable BG3",              0x0800,         11,     DisplayFormat::BOOL},
            {"Enable OBJ",              0x1000,         12,     DisplayFormat::BOOL},
            {"Enable Window 0",         0x2000,         13,     DisplayFormat::BOOL},
            {"Enable Window 1",         0x4000,         14,     DisplayFormat::BOOL},
            {"Enable OBJ Window",       0x8000,         15,     DisplayFormat::BOOL}
        }
    },

    // Green Swap
    {
        "UNDOCUMENTED",
        "Green Swap",
        0x0400'0002,
        2,
        {
            {"Green Swap",              0x0001,         0,      DisplayFormat::BOOL}
        }
    },

    // DISPSTAT
    {
        "DISPSTAT",
        "General LCD Status",
        0x0400'0004,
        2,
        {
            {"V-Blank Flag",            0x0001,         0,      DisplayFormat::BOOL},
            {"H-Blank Flag",            0x0002,         1,      DisplayFormat::BOOL},
            {"V-Counter Flag",          0x0004,         2,      DisplayFormat::BOOL},
            {"Enable V-Blank IRQ",      0x0008,         3,      DisplayFormat::BOOL},
            {"Enable H-Blank IRQ",      0x0010,         4,      DisplayFormat::BOOL},
            {"Enable V-Counter IRQ",    0x0020,         5,      DisplayFormat::BOOL},
            {"V-Count Setting",         0xFF00,         8,      DisplayFormat::DEC}
        }
    },

    // VCOUNT
    {
        "VCOUNT",
        "Vertical Counter",
        0x0400'0006,
        2,
        {
            {"Current Scanline",        0x00FF,         0,      DisplayFormat::DEC}
        }
    },

    // BG0CNT
    {
        "BG0CNT",
        "BG0 Control",
        0x0400'0008,
        2,
        {
            {"Priority",                0x0003,         0,      DisplayFormat::DEC},
            {"Character Base Block",    0x000C,         2,      DisplayFormat::DEC},
            {"Mosaic",                  0x0040,         6,      DisplayFormat::BOOL},
            {"256 Color Mode",          0x0080,         7,      DisplayFormat::BOOL},
            {"Screen Base Block",       0x1F00,         8,      DisplayFormat::DEC},
            {"Screen Size",             0xC000,         14,     DisplayFormat::DEC}
        }
    },

    // BG1CNT
    {
        "BG1CNT",
        "BG1 Control",
        0x0400'000A,
        2,
        {
            {"Priority",                0x0003,         0,      DisplayFormat::DEC},
            {"Character Base Block",    0x000C,         2,      DisplayFormat::DEC},
            {"Mosaic",                  0x0040,         6,      DisplayFormat::BOOL},
            {"256 Color Mode",          0x0080,         7,      DisplayFormat::BOOL},
            {"Screen Base Block",       0x1F00,         8,      DisplayFormat::DEC},
            {"Screen Size",             0xC000,         14,     DisplayFormat::DEC}
        }
    },

    // BG2CNT
    {
        "BG2CNT",
        "BG2 Control",
        0x0400'000C,
        2,
        {
            {"Priority",                0x0003,         0,      DisplayFormat::DEC},
            {"Character Base Block",    0x000C,         2,      DisplayFormat::DEC},
            {"Mosaic",                  0x0040,         6,      DisplayFormat::BOOL},
            {"256 Color Mode",          0x0080,         7,      DisplayFormat::BOOL},
            {"Screen Base Block",       0x1F00,         8,      DisplayFormat::DEC},
            {"Wraparound",              0x2000,         13,     DisplayFormat::BOOL},
            {"Screen Size",             0xC000,         14,     DisplayFormat::DEC}
        }
    },

    // BG3CNT
    {
        "BG3CNT",
        "BG3 Control",
        0x0400'000E,
        2,
        {
            {"Priority",                0x0003,         0,      DisplayFormat::DEC},
            {"Character Base Block",    0x000C,         2,      DisplayFormat::DEC},
            {"Mosaic",                  0x0040,         6,      DisplayFormat::BOOL},
            {"256 Color Mode",          0x0080,         7,      DisplayFormat::BOOL},
            {"Screen Base Block",       0x1F00,         8,      DisplayFormat::DEC},
            {"Wraparound",              0x2000,         13,     DisplayFormat::BOOL},
            {"Screen Size",             0xC000,         14,     DisplayFormat::DEC}
        }
    },

    // BG0HOFS
    {
        "BG0HOFS",
        "BG0 X-Offset",
        0x0400'0010,
        2,
        {
            {"Offset",                  0x01FF,         0,      DisplayFormat::DEC}
        }
    },

    // BG0VOFS
    {
        "BG0VOFS",
        "BG0 Y-Offset",
        0x0400'0012,
        2,
        {
            {"Offset",                  0x01FF,         0,      DisplayFormat::DEC}
        }
    },

    // BG1HOFS
    {
        "BG1HOFS",
        "BG1 X-Offset",
        0x0400'0014,
        2,
        {
            {"Offset",                  0x01FF,         0,      DisplayFormat::DEC}
        }
    },

    // BG1VOFS
    {
        "BG1VOFS",
        "BG1 Y-Offset",
        0x0400'0016,
        2,
        {
            {"Offset",                  0x01FF,         0,      DisplayFormat::DEC}
        }
    },

    // BG2HOFS
    {
        "BG2HOFS",
        "BG2 X-Offset",
        0x0400'0018,
        2,
        {
            {"Offset",                  0x01FF,         0,      DisplayFormat::DEC}
        }
    },

    // BG2VOFS
    {
        "BG2VOFS",
        "BG2 Y-Offset",
        0x0400'001A,
        2,
        {
            {"Offset",                  0x01FF,         0,      DisplayFormat::DEC}
        }
    },

    // BG3HOFS
    {
        "BG3HOFS",
        "BG3 X-Offset",
        0x0400'001C,
        2,
        {
            {"Offset",                  0x01FF,         0,      DisplayFormat::DEC}
        }
    },

    // BG3VOFS
    {
        "BG3VOFS",
        "BG3 Y-Offset",
        0x0400'001E,
        2,
        {
            {"Offset",                  0x01FF,         0,      DisplayFormat::DEC}
        }
    },

    // BG2PA
    {
        "BG2PA",
        "BG2 Rotation/Scaling Parameter A",
        0x0400'0020,
        2,
        {
            {"Fractional Portion",      0x00FF,         0,      DisplayFormat::DEC},
            {"Integer Portion",         0x7F00,         8,      DisplayFormat::DEC},
            {"Negative",                0x8000,         15,     DisplayFormat::BOOL},
        }
    },

    // BG2PB
    {
        "BG2PB",
        "BG2 Rotation/Scaling Parameter B",
        0x0400'0022,
        2,
        {
            {"Fractional Portion",      0x00FF,         0,      DisplayFormat::DEC},
            {"Integer Portion",         0x7F00,         8,      DisplayFormat::DEC},
            {"Negative",                0x8000,         15,     DisplayFormat::BOOL},
        }
    },

    // BG2PC
    {
        "BG2PC",
        "BG2 Rotation/Scaling Parameter C",
        0x0400'0024,
        2,
        {
            {"Fractional Portion",      0x00FF,         0,      DisplayFormat::DEC},
            {"Integer Portion",         0x7F00,         8,      DisplayFormat::DEC},
            {"Negative",                0x8000,         15,     DisplayFormat::BOOL},
        }
    },

    // BG2PD
    {
        "BG2PD",
        "BG2 Rotation/Scaling Parameter D",
        0x0400'0026,
        2,
        {
            {"Fractional Portion",      0x00FF,         0,      DisplayFormat::DEC},
            {"Integer Portion",         0x7F00,         8,      DisplayFormat::DEC},
            {"Negative",                0x8000,         15,     DisplayFormat::BOOL},
        }
    },

    // BG2X
    {
        "BG2X",
        "BG2 Reference Point X-Coordinate",
        0x0400'0028,
        4,
        {
            {"Fractional Portion",      0x0000'00FF,    0,      DisplayFormat::DEC},
            {"Integer Portion",         0x07FF'FF00,    8,      DisplayFormat::HEX},
            {"Negative",                0x0800'0000,    27,     DisplayFormat::BOOL}
        }
    },

    // BG2Y
    {
        "BG2Y",
        "BG2 Reference Point Y-Coordinate",
        0x0400'002C,
        4,
        {
            {"Fractional Portion",      0x0000'00FF,    0,      DisplayFormat::DEC},
            {"Integer Portion",         0x07FF'FF00,    8,      DisplayFormat::HEX},
            {"Negative",                0x0800'0000,    27,     DisplayFormat::BOOL}
        }
    },

    // BG3PA
    {
        "BG3PA",
        "BG3 Rotation/Scaling Parameter A",
        0x0400'0030,
        2,
        {
            {"Fractional Portion",      0x00FF,         0,      DisplayFormat::DEC},
            {"Integer Portion",         0x7F00,         8,      DisplayFormat::DEC},
            {"Negative",                0x8000,         15,     DisplayFormat::BOOL},
        }
    },

    // BG3PB
    {
        "BG3PB",
        "BG3 Rotation/Scaling Parameter B",
        0x0400'0032,
        2,
        {
            {"Fractional Portion",      0x00FF,         0,      DisplayFormat::DEC},
            {"Integer Portion",         0x7F00,         8,      DisplayFormat::DEC},
            {"Negative",                0x8000,         15,     DisplayFormat::BOOL},
        }
    },

    // BG3PC
    {
        "BG3PC",
        "BG3 Rotation/Scaling Parameter C",
        0x0400'0034,
        2,
        {
            {"Fractional Portion",      0x00FF,         0,      DisplayFormat::DEC},
            {"Integer Portion",         0x7F00,         8,      DisplayFormat::DEC},
            {"Negative",                0x8000,         15,     DisplayFormat::BOOL},
        }
    },

    // BG3PD
    {
        "BG3PD",
        "BG3 Rotation/Scaling Parameter D",
        0x0400'0036,
        2,
        {
            {"Fractional Portion",      0x00FF,         0,      DisplayFormat::DEC},
            {"Integer Portion",         0x7F00,         8,      DisplayFormat::DEC},
            {"Negative",                0x8000,         15,     DisplayFormat::BOOL},
        }
    },

    // BG3X
    {
        "BG3X",
        "BG3 Reference Point X-Coordinate",
        0x0400'0038,
        4,
        {
            {"Fractional Portion",      0x0000'00FF,    0,      DisplayFormat::DEC},
            {"Integer Portion",         0x07FF'FF00,    8,      DisplayFormat::HEX},
            {"Negative",                0x0800'0000,    27,     DisplayFormat::BOOL}
        }
    },

    // BG3Y
    {
        "BG3Y",
        "BG3 Reference Point Y-Coordinate",
        0x0400'003C,
        4,
        {
            {"Fractional Portion",      0x0000'00FF,    0,      DisplayFormat::DEC},
            {"Integer Portion",         0x07FF'FF00,    8,      DisplayFormat::HEX},
            {"Negative",                0x0800'0000,    27,     DisplayFormat::BOOL}
        }
    },

    // WIN0H
    {
        "WIN0H",
        "Window 0 Horizontal Dimensions",
        0x0400'0040,
        2,
        {
            {"X2 (right)",              0x00FF,         0,      DisplayFormat::DEC},
            {"X1 (left)",               0xFF00,         8,      DisplayFormat::DEC}
        }
    },

    // WIN1H
    {
        "WIN1H",
        "Window 1 Horizontal Dimensions",
        0x0400'0042,
        2,
        {
            {"X2 (right)",              0x00FF,         0,      DisplayFormat::DEC},
            {"X1 (left)",               0xFF00,         8,      DisplayFormat::DEC}
        }
    },

    // WIN0V
    {
        "WIN0H",
        "Window 0 Vertical Dimensions",
        0x0400'0044,
        2,
        {
            {"Y2 (bottom)",             0x00FF,         0,      DisplayFormat::DEC},
            {"Y1 (top)",                0xFF00,         8,      DisplayFormat::DEC}
        }
    },

    // WIN1V
    {
        "WIN1V",
        "Window 1 Vertical Dimensions",
        0x0400'0046,
        2,
        {
            {"Y2 (bottom)",             0x00FF,         0,      DisplayFormat::DEC},
            {"Y1 (top)",                0xFF00,         8,      DisplayFormat::DEC}
        }
    },

    // WININ
    {
        "WININ",
        "Inside Window Control",
        0x0400'0048,
        2,
        {
            {"Enable BG0 - Win 0",      0x0001,         0,      DisplayFormat::BOOL},
            {"Enable BG1 - Win 0",      0x0002,         1,      DisplayFormat::BOOL},
            {"Enable BG2 - Win 0",      0x0004,         2,      DisplayFormat::BOOL},
            {"Enable BG3 - Win 0",      0x0008,         3,      DisplayFormat::BOOL},
            {"Enable OBJ - Win 0",      0x0010,         4,      DisplayFormat::BOOL},
            {"Enable Effects - Win 0",  0x0020,         5,      DisplayFormat::BOOL},
            {"Enable BG0 - Win 1",      0x0100,         8,      DisplayFormat::BOOL},
            {"Enable BG1 - Win 1",      0x0200,         9,      DisplayFormat::BOOL},
            {"Enable BG2 - Win 1",      0x0400,         10,     DisplayFormat::BOOL},
            {"Enable BG3 - Win 1",      0x0800,         11,     DisplayFormat::BOOL},
            {"Enable OBJ - Win 1",      0x1000,         12,     DisplayFormat::BOOL},
            {"Enable Effects - Win 1",  0x2000,         13,     DisplayFormat::BOOL}
        }
    },

    // WINOUT
    {
        "WINOUT",
        "Outside/OBJ Window Control",
        0x0400'004A,
        2,
        {
            {"Enable BG0 - Out",        0x0001,         0,      DisplayFormat::BOOL},
            {"Enable BG1 - Out",        0x0002,         1,      DisplayFormat::BOOL},
            {"Enable BG2 - Out",        0x0004,         2,      DisplayFormat::BOOL},
            {"Enable BG3 - Out",        0x0008,         3,      DisplayFormat::BOOL},
            {"Enable OBJ - Out",        0x0010,         4,      DisplayFormat::BOOL},
            {"Enable Effects - Out",    0x0020,         5,      DisplayFormat::BOOL},
            {"Enable BG0 - OBJ Win",    0x0100,         8,      DisplayFormat::BOOL},
            {"Enable BG1 - OBJ Win",    0x0200,         9,      DisplayFormat::BOOL},
            {"Enable BG2 - OBJ Win",    0x0400,         10,     DisplayFormat::BOOL},
            {"Enable BG3 - OBJ Win",    0x0800,         11,     DisplayFormat::BOOL},
            {"Enable OBJ - OBJ Win",    0x1000,         12,     DisplayFormat::BOOL},
            {"Enable Effects - OBJ Win",0x2000,         13,     DisplayFormat::BOOL}
        }
    },

    // MOSAIC
    {
        "MOSAIC",
        "Mosaic Size",
        0x0400'004C,
        2,
        {
            {"BG Mosaic H-Size",        0x000F,         0,      DisplayFormat::DEC},
            {"BG Mosaic V-Size",        0x00F0,         4,      DisplayFormat::DEC},
            {"OBJ Mosaic H-Size",       0x0F00,         8,      DisplayFormat::DEC},
            {"OBJ Mosaic V-Size",       0xF000,         12,     DisplayFormat::DEC}
        }
    },

    // BLDCNT
    {
        "BLDCNT",
        "Special Effects Selection",
        0x0400'0050,
        2,
        {
            {"BG0 1st Target Pixel",    0x0001,         0,      DisplayFormat::BOOL},
            {"BG1 1st Target Pixel",    0x0002,         1,      DisplayFormat::BOOL},
            {"BG2 1st Target Pixel",    0x0004,         2,      DisplayFormat::BOOL},
            {"BG3 1st Target Pixel",    0x0008,         3,      DisplayFormat::BOOL},
            {"OBJ 1st Target Pixel",    0x0010,         4,      DisplayFormat::BOOL},
            {"BD 1st Target Pixel",     0x0020,         5,      DisplayFormat::BOOL},
            {"Color Special Effect",    0x00C0,         6,      DisplayFormat::DEC},
            {"BG0 2nd Target Pixel",    0x0100,         8,      DisplayFormat::BOOL},
            {"BG1 2nd Target Pixel",    0x0200,         9,      DisplayFormat::BOOL},
            {"BG2 2nd Target Pixel",    0x0400,         10,     DisplayFormat::BOOL},
            {"BG3 2nd Target Pixel",    0x0800,         11,     DisplayFormat::BOOL},
            {"OBJ 2nd Target Pixel",    0x1000,         12,     DisplayFormat::BOOL},
            {"BD 2nd Target Pixel",     0x2000,         13,     DisplayFormat::BOOL}
        }
    },

    // BLDALPHA
    {
        "BLDALPHA",
        "Alpha Blending Coefficients",
        0x0400'0052,
        2,
        {
            {"EVA Coefficient",         0x001F,         0,      DisplayFormat::DEC},
            {"EVB Coefficient",         0x1F00,         8,      DisplayFormat::DEC}
        }
    },

    // BLDY
    {
        "BLDY",
        "Brightness Coefficient",
        0x0400'0054,
        2,
        {
            {"EVY Coefficient",         0x001F,         0,      DisplayFormat::DEC}
        }
    },

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// APU
    ///-----------------------------------------------------------------------------------------------------------------------------
};
}  // namespace gui
