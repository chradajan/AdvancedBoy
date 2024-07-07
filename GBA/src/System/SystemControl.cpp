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

MemReadData SystemControl::ReadReg(u32 addr, AccessSize length)
{
    (void)addr;
    (void)length;
    return {1, 0, false};
}

int SystemControl::WriteReg(u32 addr, u32 val, AccessSize length)
{
    (void)addr;
    (void)val;
    (void)length;
    return 1;
}
