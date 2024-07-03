#include <GBA/include/PPU/PPU.hpp>
#include <GBA/include/Types.hpp>

namespace graphics
{
MemReadData PPU::ReadMem(Address addr, AccessSize length)
{
    (void)addr;
    (void)length;
    return {ONE_CYCLE, 0, false};
}

CpuCycles PPU::WriteMem(Address addr, u32 val, AccessSize length)
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
