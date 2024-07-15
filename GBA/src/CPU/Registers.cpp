#include <GBA/include/CPU/Registers.hpp>
#include <format>
#include <stdexcept>
#include <sstream>
#include <string>
#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/Types.hpp>

namespace cpu
{
Registers::Registers()
{
    cpsr_ = {};
    sysAndUserRegBank_ = {};
    fiqRegBank_ = {};
    supervisorRegBank_ = {};
    abortRegBank_ = {};
    irqRegBank_ = {};
    undefinedRegBank_ = {};

    SetOperatingMode(OperatingMode::Supervisor);
    SetOperatingState(OperatingState::ARM);
    SetIrqDisabled(true);
    SetFiqDisabled(true);
    SetPC(RESET_VECTOR);

    SkipBIOS();
}

u32 Registers::ReadRegister(u8 index) const
{
    return ReadRegister(index, GetOperatingMode());
}

u32 Registers::ReadRegister(u8 index, OperatingMode mode) const
{
    switch (mode)
    {
        case OperatingMode::User:
        case OperatingMode::System:
            return *sysAndUserRegLUT_.at(index);
        case OperatingMode::FIQ:
            return *fiqRegLUT_.at(index);
        case OperatingMode::IRQ:
            return *irqRegLUT_.at(index);
        case OperatingMode::Supervisor:
            return *supervisorRegLUT_.at(index);
        case OperatingMode::Abort:
            return *abortRegLUT_.at(index);
        case OperatingMode::Undefined:
            return *undefinedRegLUT.at(index);
        default:
            throw std::logic_error("Read invalid CPU register index");
    }
}

void Registers::WriteRegister(u8 index, u32 val)
{
    WriteRegister(index, val, GetOperatingMode());
}

void Registers::WriteRegister(u8 index, u32 val, OperatingMode mode)
{
    if (index == PC_INDEX)
    {
        u32 mask = InArmState() ? 0xFFFF'FFFC : 0xFFFF'FFFE;
        val &= mask;
    }

    switch (mode)
    {
        case OperatingMode::User:
        case OperatingMode::System:
            *sysAndUserRegLUT_.at(index) = val;
            break;
        case OperatingMode::FIQ:
            *fiqRegLUT_.at(index) = val;
            break;
        case OperatingMode::IRQ:
            *irqRegLUT_.at(index) = val;
            break;
        case OperatingMode::Supervisor:
            *supervisorRegLUT_.at(index) = val;
            break;
        case OperatingMode::Abort:
            *abortRegLUT_.at(index) = val;
            break;
        case OperatingMode::Undefined:
            *undefinedRegLUT.at(index) = val;
            break;
        default:
            throw std::logic_error("Wrote invalid CPU register index");
    }
}

u32 Registers::GetSPSR() const
{
    switch (GetOperatingMode())
    {
        case OperatingMode::FIQ:
            return std::bit_cast<u32>(fiqRegBank_.spsr);
        case OperatingMode::IRQ:
            return std::bit_cast<u32>(irqRegBank_.spsr);
        case OperatingMode::Supervisor:
            return std::bit_cast<u32>(supervisorRegBank_.spsr);
        case OperatingMode::Abort:
            return std::bit_cast<u32>(abortRegBank_.spsr);
        case OperatingMode::Undefined:
            return std::bit_cast<u32>(undefinedRegBank_.spsr);
        default:
            return std::bit_cast<u32>(cpsr_);
    }
}

void Registers::SetSPSR(u32 val)
{
    switch (GetOperatingMode())
    {
        case OperatingMode::FIQ:
            fiqRegBank_.spsr = std::bit_cast<CPSR>(val);
            break;
        case OperatingMode::IRQ:
            irqRegBank_.spsr = std::bit_cast<CPSR>(val);
            break;
        case OperatingMode::Supervisor:
            supervisorRegBank_.spsr = std::bit_cast<CPSR>(val);
            break;
        case OperatingMode::Abort:
            abortRegBank_.spsr = std::bit_cast<CPSR>(val);
            break;
        case OperatingMode::Undefined:
            undefinedRegBank_.spsr = std::bit_cast<CPSR>(val);
            break;
        default:
            break;
    }
}

void Registers::LoadSPSR()
{
    switch (GetOperatingMode())
    {
        case OperatingMode::FIQ:
            cpsr_ = fiqRegBank_.spsr;
            break;
        case OperatingMode::IRQ:
            cpsr_ = irqRegBank_.spsr;
            break;
        case OperatingMode::Supervisor:
            cpsr_ = supervisorRegBank_.spsr;
            break;
        case OperatingMode::Abort:
            cpsr_ = abortRegBank_.spsr;
            break;
        case OperatingMode::Undefined:
            cpsr_ = undefinedRegBank_.spsr;
            break;
        default:
            break;
    }
}

std::string Registers::RegistersString() const
{
    std::stringstream regStream;

    for (int i = 0; i < 16; ++i)
    {
        regStream << std::format("R{} {:08X}  ", i, ReadRegister(i));
    }

    regStream << "CPSR: " << (IsNegative() ? "N" : "-") <<
                             (IsZero() ? "Z" : "-") <<
                             (IsCarry() ? "C" : "-") <<
                             (IsOverflow() ? "V" : "-") << "  ";
    regStream << (IsIrqDisabled() ? "I" : "-") << (IsFiqDisabled() ? "F" : "-") << (InThumbState() ? "T" : "-") << "  " << "Mode: ";
    u32 spsr = GetSPSR();

    switch (GetOperatingMode())
    {
        case OperatingMode::User:
            regStream << "User";
            break;
        case OperatingMode::FIQ:
            regStream << std::format("FIQ         SPSR: {:08X}", spsr);
            break;
        case OperatingMode::IRQ:
            regStream << std::format("IRQ         SPSR: {:08X}", spsr);
            break;
        case OperatingMode::Supervisor:
            regStream << std::format("Supervisor  SPSR: {:08X}", spsr);
            break;
        case OperatingMode::Abort:
            regStream << std::format("Abort       SPSR: {:08X}", spsr);
            break;
        case OperatingMode::System:
            regStream << "System";
            break;
        case OperatingMode::Undefined:
            regStream << std::format("Undefined   SPSR: {:08X}", spsr);
            break;
    }

    return regStream.str();
}

void Registers::SkipBIOS()
{
    SetOperatingMode(OperatingMode::System);
    SetPC(0x0800'0000);
    WriteRegister(SP_INDEX, 0x0300'7F00, OperatingMode::System);
    WriteRegister(SP_INDEX, 0x0300'7FA0, OperatingMode::IRQ);
    WriteRegister(SP_INDEX, 0x0300'7FE0, OperatingMode::Supervisor);
}
}  // namespace cpu
