#include <GBA/include/Keypad/Keypad.hpp>
#include <array>
#include <cstddef>
#include <GBA/include/Keypad/Registers.hpp>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Types/Types.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>

Keypad::Keypad(SystemControl& systemControl) : systemControl_(systemControl)
{
    registers_.fill(std::byte{0});
}

void Keypad::UpdateKeypad(KEYINPUT keyinput)
{
    std::memcpy(&registers_[KEYINPUT::STATUS_INDEX], &keyinput, sizeof(KEYINPUT));
    CheckKeypadIRQ();
}

MemReadData Keypad::ReadReg(u32 addr, AccessSize length)
{
    u32 val = ReadMemoryBlock(registers_, addr, KEYPAD_IO_ADDR_MIN, length);
    return {1, val, false};
}

int Keypad::WriteReg(u32 addr, u32 val, AccessSize length)
{
    KEYINPUT keyinput;
    std::memcpy(&keyinput, &registers_[KEYINPUT::STATUS_INDEX], sizeof(KEYINPUT));
    WriteMemoryBlock(registers_, addr, KEYPAD_IO_ADDR_MIN, val, length);
    std::memcpy(&registers_[KEYINPUT::STATUS_INDEX], &keyinput, sizeof(KEYINPUT));
    CheckKeypadIRQ();
    return 1;
}

void Keypad::CheckKeypadIRQ()
{
    KEYINPUT keycnt;
    std::memcpy(&keycnt, &registers_[KEYINPUT::CONTROL_INDEX], sizeof(KEYINPUT));

    if (!keycnt.IRQ)
    {
        return;
    }

    u16 keyinputRaw;
    std::memcpy(&keyinputRaw, &registers_[KEYINPUT::STATUS_INDEX], sizeof(u16));

    u16 keycntRaw;
    std::memcpy(&keycntRaw, &registers_[KEYINPUT::CONTROL_INDEX], sizeof(u16));

    keyinputRaw &= KEYINPUT::BUTTON_MASK;
    keycntRaw &= KEYINPUT::BUTTON_MASK;

    bool irq = false;

    if (keycnt.Cond)
    {
        // Logical AND
        irq = (keyinputRaw & keycntRaw) == keycntRaw;
    }
    else
    {
        // Logical OR
        irq = (keyinputRaw & keycntRaw) != 0;
    }

    if (irq)
    {
        systemControl_.RequestInterrupt(InterruptType::KEYPAD);
    }
}
