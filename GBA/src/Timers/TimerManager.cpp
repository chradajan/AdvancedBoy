#include <GBA/include/Timers/TimerManager.hpp>
#include <array>
#include <cstddef>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Types.hpp>

namespace timers
{
TimerManager::TimerManager(EventScheduler& scheduler, SystemControl& systemControl) :
    scheduler_(scheduler),
    systemControl_(systemControl)
{
    registers_.fill(std::byte{0});
}

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
