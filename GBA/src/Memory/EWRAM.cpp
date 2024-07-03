#include <GBA/include/Memory/EWRAM.hpp>
#include <GBA/include/Types.hpp>

MemReadData EWRAM::ReadMem(Address addr, AccessSize length)
{
    (void)addr;
    (void)length;
    return {ONE_CYCLE, 0, false};
}

CpuCycles EWRAM::WriteMem(Address addr, u32 val, AccessSize length)
{
    (void)addr;
    (void)length;
    (void)val;
    return ONE_CYCLE;
}
