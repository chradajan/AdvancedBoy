#include <GBA/include/PPU/PPU.hpp>
#include <array>
#include <cstddef>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Types.hpp>

namespace graphics
{
PPU::PPU(EventScheduler& scheduler, SystemControl& systemControl) : scheduler_(scheduler), systemControl_(systemControl)
{
    PRAM_.fill(std::byte{0});
    OAM_.fill(std::byte{0});
    VRAM_.fill(std::byte{0});
    registers_.fill(std::byte{0});
}

MemReadData PPU::ReadPRAM(u32 addr, AccessSize length)
{
    (void)addr;
    (void)length;
    return {1, 0, false};
}

int PPU::WritePRAM(u32 addr, u32 val, AccessSize length)
{
    (void)addr;
    (void)length;
    (void)val;
    return 1;
}

MemReadData PPU::ReadOAM(u32 addr, AccessSize length)
{
    (void)addr;
    (void)length;
    return {1, 0, false};
}

int PPU::WriteOAM(u32 addr, u32 val, AccessSize length)
{
    (void)addr;
    (void)length;
    (void)val;
    return 1;
}

MemReadData PPU::ReadVRAM(u32 addr, AccessSize length)
{
    (void)addr;
    (void)length;
    return {1, 0, false};
}

int PPU::WriteVRAM(u32 addr, u32 val, AccessSize length)
{
    (void)addr;
    (void)length;
    (void)val;
    return 1;
}

MemReadData PPU::ReadReg(u32 addr, AccessSize length)
{
    (void)addr;
    (void)length;
    return {1, 0, false};
}

int PPU::WriteReg(u32 addr, u32 val, AccessSize length)
{
    (void)addr;
    (void)val;
    (void)length;
    return 1;
}
}  // namespace graphics
