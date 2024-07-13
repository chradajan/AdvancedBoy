#include <GBA/include/CPU/ARM7TDMI.hpp>
#include <cstring>
#include <stdexcept>
#include <GBA/include/CPU/ARM.hpp>
#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/Types.hpp>

namespace cpu
{
using namespace arm;

void ARM7TDMI::DecodeAndExecuteARM(u32 instruction, bool log)
{
    u8 conditionCode = (instruction & 0xF000'0000) >> 24;
    bool conditionMet = (conditionCode == 0x0E) || ConditionSatisfied(conditionCode);

    if (BranchAndExchange::IsInstanceOf(instruction))
    {
        if (log) LogBranchAndExchange(instruction);
        if (conditionMet) ExecuteBranchAndExchange(instruction);
    }
    else if (BlockDataTransfer::IsInstanceOf(instruction))
    {
        if (log) LogBlockDataTransfer(instruction);
        if (conditionMet) ExecuteBlockDataTransfer(instruction);
    }
    else if (Branch::IsInstanceOf(instruction))
    {
        if (log) LogBranch(instruction);
        if (conditionMet) ExecuteBranch(instruction);
    }
    else if (SoftwareInterrupt::IsInstanceOf(instruction))
    {
        if (log) LogArmSoftwareInterrupt(instruction);
        if (conditionMet) ExecuteArmSoftwareInterrupt(instruction);
    }
    else if (Undefined::IsInstanceOf(instruction))
    {
        if (log) LogUndefined(instruction);
        if (conditionMet) ExecuteUndefined(instruction);
    }
    else if (SingleDataTransfer::IsInstanceOf(instruction))
    {
        if (log) LogSingleDataTransfer(instruction);
        if (conditionMet) ExecuteSingleDataTransfer(instruction);
    }
    else if (SingleDataSwap::IsInstanceOf(instruction))
    {
        if (log) LogSingleDataSwap(instruction);
        if (conditionMet) ExecuteSingleDataSwap(instruction);
    }
    else if (Multiply::IsInstanceOf(instruction))
    {
        if (log) LogMultiply(instruction);
        if (conditionMet) ExecuteMultiply(instruction);
    }
    else if (MultiplyLong::IsInstanceOf(instruction))
    {
        if (log) LogMultiplyLong(instruction);
        if (conditionMet) ExecuteMultiplyLong(instruction);
    }
    else if (HalfwordDataTransferRegOffset::IsInstanceOf(instruction))
    {
        if (log) LogHalfwordDataTransferRegOffset(instruction);
        if (conditionMet) ExecuteHalfwordDataTransferRegOffset(instruction);
    }
    else if (HalfwordDataTransferImmOffset::IsInstanceOf(instruction))
    {
        if (log) LogHalfwordDataTransferImmOffset(instruction);
        if (conditionMet) ExecuteHalfwordDataTransferImmOffset(instruction);
    }
    else if (PSRTransferMRS::IsInstanceOf(instruction))
    {
        if (log) LogPSRTransferMRS(instruction);
        if (conditionMet) ExecutePSRTransferMRS(instruction);
    }
    else if (PSRTransferMSR::IsInstanceOf(instruction))
    {
        if (log) LogPSRTransferMSR(instruction);
        if (conditionMet) ExecutePSRTransferMSR(instruction);
    }
    else
    {
        throw std::runtime_error("Unable to decode ARM instruction");
    }
}

void ARM7TDMI::ExecuteBranchAndExchange(u32 instruction)
{
    (void)instruction;
    throw std::runtime_error("ExecuteBranchAndExchange not implemented");
}

void ARM7TDMI::ExecuteBlockDataTransfer(u32 instruction)
{
    (void)instruction;
    throw std::runtime_error("BlockDataTransfer not implemented");
}

void ARM7TDMI::ExecuteBranch(u32 instruction)
{
    (void)instruction;
    throw std::runtime_error("Branch not implemented");
}

void ARM7TDMI::ExecuteArmSoftwareInterrupt(u32 instruction)
{
    (void)instruction;
    throw std::runtime_error("ArmSoftwareInterrupt not implemented");
}

void ARM7TDMI::ExecuteUndefined(u32 instruction)
{
    (void)instruction;
    throw std::runtime_error("Undefined not implemented");
}

void ARM7TDMI::ExecuteSingleDataTransfer(u32 instruction)
{
    (void)instruction;
    throw std::runtime_error("SingleDataTransfer not implemented");
}

void ARM7TDMI::ExecuteSingleDataSwap(u32 instruction)
{
    (void)instruction;
    throw std::runtime_error("SingleDataSwap not implemented");
}

void ARM7TDMI::ExecuteMultiply(u32 instruction)
{
    (void)instruction;
    throw std::runtime_error("Multiply not implemented");
}

void ARM7TDMI::ExecuteMultiplyLong(u32 instruction)
{
    (void)instruction;
    throw std::runtime_error("MultiplyLong not implemented");
}

void ARM7TDMI::ExecuteHalfwordDataTransferRegOffset(u32 instruction)
{
    (void)instruction;
    throw std::runtime_error("HalfwordDataTransferRegOffset not implemented");
}

void ARM7TDMI::ExecuteHalfwordDataTransferImmOffset(u32 instruction)
{
    (void)instruction;
    throw std::runtime_error("HalfwordDataTransferImmOffset not implemented");
}

void ARM7TDMI::ExecutePSRTransferMRS(u32 instruction)
{
    (void)instruction;
    throw std::runtime_error("PSRTransferMRS not implemented");
}

void ARM7TDMI::ExecutePSRTransferMSR(u32 instruction)
{
    (void)instruction;
    throw std::runtime_error("PSRTransferMSR not implemented");
}

void ARM7TDMI::ExecuteDataProcessing(u32 instruction)
{
    (void)instruction;
    throw std::runtime_error("DataProcessing not implemented");
}
}  // namespace cpu::arm
