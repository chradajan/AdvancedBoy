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

MemReadData APU::ReadReg(u32 addr, AccessSize length)
{
    (void)addr;
    (void)length;
    return {1, 0, false};
}

int APU::WriteReg(u32 addr, u32 val, AccessSize length)
{
    (void)addr;
    (void)val;
    (void)length;
    return 1;
}
}  // namespace audio
