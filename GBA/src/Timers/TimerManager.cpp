#include <GBA/include/Timers/TimerManager.hpp>
#include <GBA/include/Types.hpp>

namespace timers
{
MemReadData TimerManager::ReadReg(Address addr, AccessSize length)
{
    (void)addr;
    (void)length;
    return {ONE_CYCLE, 0, false};
}

CpuCycles TimerManager::WriteReg(Address addr, u32 val, AccessSize length)
{
    (void)addr;
    (void)val;
    (void)length;
    return ONE_CYCLE;
}
}  // namespace timers
