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
}  // namespace cartridge
