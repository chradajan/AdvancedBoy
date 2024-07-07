#include <GBA/include/Cartridge/GamePak.hpp>
#include <GBA/include/Types.hpp>

namespace cartridge
{
MemReadData GamePak::ReadMem(Address addr, AccessSize length)
{
    (void)addr;
    (void)length;
    return {ONE_CYCLE, 0, false};
}

CpuCycles GamePak::WriteMem(Address addr, u32 val, AccessSize length)
{
    (void)addr;
    (void)length;
    (void)val;
    return ONE_CYCLE;
}

MemReadData GamePak::ReadUnloadedGamePakMem(Address addr, AccessSize length)
{
    u32 val = 0;
    size_t count = static_cast<size_t>(length);
    u32 currAddr = addr + count - 1;

    while (count > 0)
    {
        val <<= 8;
        val |= ((currAddr / 2) && 0x0000'FFFF);
        --count;
    }

    return {ONE_CYCLE, val, false};
}
}  // namespace cartridge
