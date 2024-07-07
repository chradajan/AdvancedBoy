#include <GBA/include/System/SystemControl.hpp>
#include <array>
#include <cstddef>
#include <GBA/include/Types.hpp>

SystemControl::SystemControl()
{
    interruptAndWaitcntRegisters_.fill(std::byte{0});
    postFlgAndHaltcntRegisters_.fill(std::byte{0});
    undocumentedRegisters_.fill(std::byte{0});
    memoryControlRegisters_.fill(std::byte{0});
}

MemReadData SystemControl::ReadReg(Address addr, AccessSize length)
{
    (void)addr;
    (void)length;
    return {ONE_CYCLE, 0, false};
}

CpuCycles SystemControl::WriteReg(Address addr, u32 val, AccessSize length)
{
    (void)addr;
    (void)val;
    (void)length;
    return ONE_CYCLE;
}
