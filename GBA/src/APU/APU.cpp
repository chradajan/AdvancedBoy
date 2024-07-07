#include <GBA/include/APU/APU.hpp>
#include <array>
#include <cstddef>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/Types.hpp>

namespace audio
{
APU::APU(EventScheduler& scheduler) : scheduler_(scheduler)
{
    registers_.fill(std::byte{0});
}

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
