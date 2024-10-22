#include <GBA/include/CPU/Registers.hpp>
#include <bit>
#include <format>
#include <fstream>
#include <stdexcept>
#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/Debug/DebugTypes.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>
#include <GBA/include/Utilities/Types.hpp>

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

void Registers::Serialize(std::ofstream& saveState) const
{
    SerializeTrivialType(cpsr_);
    SerializeTrivialType(sysAndUserRegBank_);
    SerializeTrivialType(fiqRegBank_);
    SerializeTrivialType(supervisorRegBank_);
    SerializeTrivialType(abortRegBank_);
    SerializeTrivialType(irqRegBank_);
    SerializeTrivialType(undefinedRegBank_);
}

void Registers::Deserialize(std::ifstream& saveState)
{
    DeserializeTrivialType(cpsr_);
    DeserializeTrivialType(sysAndUserRegBank_);
    DeserializeTrivialType(fiqRegBank_);
    DeserializeTrivialType(supervisorRegBank_);
    DeserializeTrivialType(abortRegBank_);
    DeserializeTrivialType(irqRegBank_);
    DeserializeTrivialType(undefinedRegBank_);
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
