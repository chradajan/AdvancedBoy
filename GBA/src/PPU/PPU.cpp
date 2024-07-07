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

MemReadData PPU::ReadPRAM(Address addr, AccessSize length)
{
    (void)addr;
    (void)length;
    return {ONE_CYCLE, 0, false};
}

CpuCycles PPU::WritePRAM(Address addr, u32 val, AccessSize length)
{
    (void)addr;
    (void)length;
    (void)val;
    return ONE_CYCLE;
}

MemReadData PPU::ReadOAM(Address addr, AccessSize length)
{
    (void)addr;
    (void)length;
    return {ONE_CYCLE, 0, false};
}

CpuCycles PPU::WriteOAM(Address addr, u32 val, AccessSize length)
{
    (void)addr;
    (void)length;
    (void)val;
    return ONE_CYCLE;
}

MemReadData PPU::ReadVRAM(Address addr, AccessSize length)
{
    (void)addr;
    (void)length;
    return {ONE_CYCLE, 0, false};
}

CpuCycles PPU::WriteVRAM(Address addr, u32 val, AccessSize length)
{
    (void)addr;
    (void)length;
    (void)val;
    return ONE_CYCLE;
}

MemReadData PPU::ReadReg(Address addr, AccessSize length)
{
    (void)addr;
    (void)length;
    return {ONE_CYCLE, 0, false};
}

CpuCycles PPU::WriteReg(Address addr, u32 val, AccessSize length)
{
    (void)addr;
    (void)val;
    (void)length;
    return ONE_CYCLE;
}
}  // namespace graphics
