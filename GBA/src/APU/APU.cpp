#include <GBA/include/APU/APU.hpp>
#include <GBA/include/Types.hpp>

namespace audio
{
MemReadData APU::ReadReg(Address addr, AccessSize length)
{
    (void)addr;
    (void)length;
    return {ONE_CYCLE, 0, false};
}

CpuCycles APU::WriteReg(Address addr, u32 val, AccessSize length)
{
    (void)addr;
    (void)val;
    (void)length;
    return ONE_CYCLE;
}
}  // namespace audio
