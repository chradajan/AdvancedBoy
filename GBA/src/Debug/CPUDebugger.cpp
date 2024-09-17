#include <GBA/include/Debug/CPUDebugger.hpp>
#include <unordered_map>
#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/CPU/Registers.hpp>
#include <GBA/include/Debug/ArmDisassembler.hpp>
#include <GBA/include/Debug/ThumbDisassembler.hpp>
#include <GBA/include/Utilities/CircularBuffer.hpp>

namespace debug
{
void CPUDebugger::PopulateCpuRegState(RegState& regState) const
{
    auto mode = cpu_.registers_.GetOperatingMode();
    regState.mode = static_cast<u8>(mode);

    for (u8 i = 0; i < 16; ++i)
    {
        regState.registers[i] = cpu_.registers_.ReadRegister(i, mode);
    }

    regState.cpsr = std::bit_cast<u32, cpu::Registers::CPSR>(cpu_.registers_.cpsr_);

    if ((mode == cpu::OperatingMode::User) || (mode == cpu::OperatingMode::System))
    {
        regState.spsr = {};
    }
    else
    {
        regState.spsr = cpu_.registers_.GetSPSR();
    }

    regState.negative = cpu_.registers_.IsNegative();
    regState.zero = cpu_.registers_.IsZero();
    regState.carry = cpu_.registers_.IsCarry();
    regState.overflow = cpu_.registers_.IsOverflow();

    regState.irqDisable = cpu_.registers_.IsIrqDisabled();
    regState.fiqDisable = cpu_.registers_.IsFiqDisabled();
    regState.thumbState = cpu_.registers_.InThumbState();
}

Mnemonic const& CPUDebugger::DisassembleArmInstruction(u32 instruction)
{
    auto foundIter = decodedArmInstructions_.find(instruction);

    if (foundIter == decodedArmInstructions_.end())
    {
        bool inserted;
        auto mnemonic = ::cpu::arm::DisassembleInstruction(instruction);
        std::tie(foundIter, inserted) = decodedArmInstructions_.insert({instruction, mnemonic});
    }

    return foundIter->second;
}

Mnemonic const& CPUDebugger::DisassembleThumbInstruction(u16 instruction)
{
    auto foundIter = decodedThumbInstructions_.find(instruction);

    if (foundIter == decodedThumbInstructions_.end())
    {
        bool inserted;
        auto mnemonic = ::cpu::thumb::DisassembleInstruction(instruction);
        std::tie(foundIter, inserted) = decodedThumbInstructions_.insert({instruction, mnemonic});
    }

    return foundIter->second;
}
}  // namespace debug
