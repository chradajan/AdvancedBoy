#include <GBA/include/CPU/Registers.hpp>
#include <bit>
#include <format>
#include <fstream>
#include <stdexcept>
#include <utility>
#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/Debug/DebugTypes.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>
#include <GBA/include/Utilities/Types.hpp>

namespace cpu
{
Registers::Registers(bool skipBiosIntro)
{
    ZeroObject(cpsr_);
    ZeroObject(spsr_);
    gpRegisters_.fill(0);
    fiqRegisters_.fill(0);
    svcRegisters_.fill(0);
    abtRegisters_.fill(0);
    irqRegisters_.fill(0);
    undRegisters_.fill(0);

    cpsr_.Mode = static_cast<u32>(OperatingMode::Supervisor);
    SetOperatingState(OperatingState::ARM);
    SetIrqDisabled(true);
    SetFiqDisabled(true);
    SetPC(RESET_VECTOR);

    if (skipBiosIntro)
    {
        SkipBIOS();
    }
}

u32 Registers::ReadRegister(u8 index, OperatingMode mode) const
{
    auto const currMode = GetOperatingMode();

    if ((currMode == mode) || (index == PC_INDEX) || (index < 8))
    {
        return gpRegisters_[index];
    }

    switch (mode)
    {
        case OperatingMode::User:
        case OperatingMode::System:
        {
            if ((currMode == OperatingMode::User) || (currMode == OperatingMode::System))
            {
                return gpRegisters_[index];
            }
            else if (currMode == OperatingMode::FIQ)
            {
                return fiqRegisters_[index - 8];
            }
            else if (currMode == OperatingMode::Supervisor)
            {
                return (index < 13) ? gpRegisters_[index] : svcRegisters_[index - 13];
            }
            else if (currMode == OperatingMode::Abort)
            {
                return (index < 13) ? gpRegisters_[index] : abtRegisters_[index - 13];
            }
            else if (currMode == OperatingMode::IRQ)
            {
                return (index < 13) ? gpRegisters_[index] : irqRegisters_[index - 13];
            }
            else
            {
                return (index < 13) ? gpRegisters_[index] : undRegisters_[index - 13];
            }
        }
        case OperatingMode::FIQ:
            return fiqRegisters_[index - 8];
        case OperatingMode::Supervisor:
            return ((index == 13) || (index == 14)) ? svcRegisters_[index - 13] : gpRegisters_[index];
        case OperatingMode::Abort:
            return ((index == 13) || (index == 14)) ? abtRegisters_[index - 13] : gpRegisters_[index];
        case OperatingMode::IRQ:
            return ((index == 13) || (index == 14)) ? irqRegisters_[index - 13] : gpRegisters_[index];
        case OperatingMode::Undefined:
            return ((index == 13) || (index == 14)) ? undRegisters_[index - 13] : gpRegisters_[index];
        default:
            throw std::logic_error("Read invalid CPU register index");
    }
}

void Registers::WriteRegister(u8 index, u32 val)
{
    if (index == PC_INDEX)
    {
        u32 mask = InArmState() ? 0xFFFF'FFFC : 0xFFFF'FFFE;
        val &= mask;
    }

    gpRegisters_[index] = val;
}

void Registers::WriteRegister(u8 index, u32 val, OperatingMode mode)
{
    if (index == PC_INDEX)
    {
        u32 mask = InArmState() ? 0xFFFF'FFFC : 0xFFFF'FFFE;
        val &= mask;
    }

    auto currMode = GetOperatingMode();

    if ((currMode == mode) || (index == PC_INDEX) || (index < 8))
    {
        gpRegisters_[index] = val;
        return;
    }

    switch (mode)
    {
        case OperatingMode::User:
        case OperatingMode::System:
        {
            if ((currMode == OperatingMode::User) || (currMode == OperatingMode::System))
            {
                gpRegisters_[index] = val;
            }
            else if (currMode == OperatingMode::FIQ)
            {
                fiqRegisters_[index - 8] = val;
            }
            else if (currMode == OperatingMode::Supervisor)
            {
                if (index < 13)
                    gpRegisters_[index] = val;
                else
                    svcRegisters_[index - 13] = val;
            }
            else if (currMode == OperatingMode::Abort)
            {
                if (index < 13)
                    gpRegisters_[index] = val;
                else
                    abtRegisters_[index - 13] = val;
            }
            else if (currMode == OperatingMode::IRQ)
            {
                if (index < 13)
                    gpRegisters_[index] = val;
                else
                    irqRegisters_[index - 13] = val;
            }
            else
            {
                if (index < 13)
                    gpRegisters_[index] = val;
                else
                    undRegisters_[index - 13] = val;
            }

            break;
        }
        case OperatingMode::FIQ:
            fiqRegisters_[index - 8] = val;
            break;
        case OperatingMode::Supervisor:
        {
            if ((index == 13) || (index == 14))
                svcRegisters_[index - 13] = val;
            else
                gpRegisters_[index] = val;

            break;
        }
        case OperatingMode::Abort:
        {
            if ((index == 13) || (index == 14))
                abtRegisters_[index - 13] = val;
            else
                gpRegisters_[index] = val;

            break;
        }
        case OperatingMode::IRQ:
        {
            if ((index == 13) || (index == 14))
                irqRegisters_[index - 13] = val;
            else
                gpRegisters_[index] = val;

            break;
        }
        case OperatingMode::Undefined:
        {
            if ((index == 13) || (index == 14))
                undRegisters_[index - 13] = val;
            else
                gpRegisters_[index] = val;

            break;
        }
        default:
            throw std::logic_error("Wrote invalid CPU register index");
    }
}

void Registers::SetOperatingMode(OperatingMode mode)
{
    auto const currMode = GetOperatingMode();

    if (currMode == mode)
    {
        return;
    }

    // If currently in an operating mode with banked registers, swap them out of the general purpose register array and back into
    // their respective array of banked registers. Also swap out the current value of SPSR.

    u32* gpRegPtr = nullptr;
    u32* bankRegPtr = nullptr;
    u8 swapCount = 0;

    switch (currMode)
    {
        case OperatingMode::FIQ:
            gpRegPtr = &gpRegisters_[8];
            bankRegPtr = &fiqRegisters_[0];
            fiqRegisters_[7] = std::bit_cast<u32>(spsr_);
            swapCount = 7;
            break;
        case OperatingMode::Supervisor:
            gpRegPtr = &gpRegisters_[13];
            bankRegPtr = &svcRegisters_[0];
            svcRegisters_[2] = std::bit_cast<u32>(spsr_);
            swapCount = 2;
            break;
        case OperatingMode::Abort:
            gpRegPtr = &gpRegisters_[13];
            bankRegPtr = &abtRegisters_[0];
            abtRegisters_[2] = std::bit_cast<u32>(spsr_);
            swapCount = 2;
            break;
        case OperatingMode::IRQ:
            gpRegPtr = &gpRegisters_[13];
            bankRegPtr = &irqRegisters_[0];
            irqRegisters_[2] = std::bit_cast<u32>(spsr_);
            swapCount = 2;
            break;
        case OperatingMode::Undefined:
            gpRegPtr = &gpRegisters_[13];
            bankRegPtr = &undRegisters_[0];
            undRegisters_[2] = std::bit_cast<u32>(spsr_);
            swapCount = 2;
            break;
        default:
            break;
    }

    for (u8 i = 0; i < swapCount; ++i)
    {
        std::swap(gpRegPtr[i], bankRegPtr[i]);
    }

    spsr_ = cpsr_;

    // Registers have now been swapped mirroring their setup in System/User mode. Swap again based on the new operating mode.

    gpRegPtr = nullptr;
    bankRegPtr = nullptr;
    swapCount = 0;

    switch (mode)
    {
        case OperatingMode::FIQ:
            gpRegPtr = &gpRegisters_[8];
            bankRegPtr = &fiqRegisters_[0];
            spsr_ = std::bit_cast<CPSR>(fiqRegisters_[7]);
            swapCount = 7;
            break;
        case OperatingMode::Supervisor:
            gpRegPtr = &gpRegisters_[13];
            bankRegPtr = &svcRegisters_[0];
            spsr_ = std::bit_cast<CPSR>(svcRegisters_[2]);
            swapCount = 2;
            break;
        case OperatingMode::Abort:
            gpRegPtr = &gpRegisters_[13];
            bankRegPtr = &abtRegisters_[0];
            spsr_ = std::bit_cast<CPSR>(abtRegisters_[2]);
            swapCount = 2;
            break;
        case OperatingMode::IRQ:
            gpRegPtr = &gpRegisters_[13];
            bankRegPtr = &irqRegisters_[0];
            spsr_ = std::bit_cast<CPSR>(irqRegisters_[2]);
            swapCount = 2;
            break;
        case OperatingMode::Undefined:
            gpRegPtr = &gpRegisters_[13];
            bankRegPtr = &undRegisters_[0];
            spsr_ = std::bit_cast<CPSR>(undRegisters_[2]);
            swapCount = 2;
            break;
        default:
            break;
    }

    for (u8 i = 0; i < swapCount; ++i)
    {
        std::swap(gpRegPtr[i], bankRegPtr[i]);
    }

    cpsr_.Mode = static_cast<u32>(mode);
}

void Registers::SetCPSR(u32 val)
{
    auto newCpsr = std::bit_cast<CPSR>(val);

    if (newCpsr.Mode != cpsr_.Mode)
    {
        SetOperatingMode(OperatingMode{newCpsr.Mode});
    }

    cpsr_ = newCpsr;
}

void Registers::Serialize(std::ofstream& saveState) const
{
    SerializeTrivialType(cpsr_);
    SerializeTrivialType(spsr_);
    SerializeArray(gpRegisters_);
    SerializeArray(fiqRegisters_);
    SerializeArray(svcRegisters_);
    SerializeArray(abtRegisters_);
    SerializeArray(irqRegisters_);
    SerializeArray(undRegisters_);
}

void Registers::Deserialize(std::ifstream& saveState)
{
    DeserializeTrivialType(cpsr_);
    DeserializeTrivialType(spsr_);
    DeserializeArray(gpRegisters_);
    DeserializeArray(fiqRegisters_);
    DeserializeArray(svcRegisters_);
    DeserializeArray(abtRegisters_);
    DeserializeArray(irqRegisters_);
    DeserializeArray(undRegisters_);
}

void Registers::SkipBIOS()
{
    cpsr_.Mode = std::bit_cast<u32>(OperatingMode::System);
    spsr_ = cpsr_;
    SetPC(0x0800'0000);
    WriteRegister(SP_INDEX, 0x0300'7F00, OperatingMode::System);
    WriteRegister(SP_INDEX, 0x0300'7FA0, OperatingMode::IRQ);
    WriteRegister(SP_INDEX, 0x0300'7FE0, OperatingMode::Supervisor);
}
}  // namespace cpu
