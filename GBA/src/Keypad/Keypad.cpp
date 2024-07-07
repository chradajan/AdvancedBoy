#include <GBA/include/Keypad/Keypad.hpp>
#include <array>
#include <cstddef>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Types.hpp>

Keypad::Keypad(SystemControl& systemControl) : systemControl_(systemControl)
{
    registers_.fill(std::byte{0});
}

MemReadData Keypad::ReadReg(Address addr, AccessSize length)
{
    (void)addr;
    (void)length;
    return {ONE_CYCLE, 0, false};
}

CpuCycles Keypad::WriteReg(Address addr, u32 val, AccessSize length)
{
    (void)addr;
    (void)val;
    (void)length;
    return ONE_CYCLE;
}
