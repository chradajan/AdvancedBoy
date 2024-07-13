#include <GBA/include/CPU/ARM7TDMI.hpp>
#include <cstring>
#include <stdexcept>
#include <GBA/include/CPU/THUMB.hpp>
#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/Types.hpp>

namespace cpu
{
using namespace thumb;

void ARM7TDMI::DecodeAndExecuteTHUMB(u16 instruction, bool log)
{
    if (SoftwareInterrupt::IsInstanceOf(instruction))
    {
        if (log) LogThumbSoftwareInterrupt(instruction);
        ExecuteThumbSoftwareInterrupt(instruction);
    }
    else if (UnconditionalBranch::IsInstanceOf(instruction))
    {
        if (log) LogUnconditionalBranch(instruction);
        ExecuteUnconditionalBranch(instruction);
    }
    else if (ConditionalBranch::IsInstanceOf(instruction))
    {
        if (log) LogConditionalBranch(instruction);
        ExecuteConditionalBranch(instruction);
    }
    else if (MultipleLoadStore::IsInstanceOf(instruction))
    {
        if (log) LogMultipleLoadStore(instruction);
        ExecuteMultipleLoadStore(instruction);
    }
    else if (LongBranchWithLink::IsInstanceOf(instruction))
    {
        if (log) LogLongBranchWithLink(instruction);
        ExecuteLongBranchWithLink(instruction);
    }
    else if (AddOffsetToStackPointer::IsInstanceOf(instruction))
    {
        if (log) LogAddOffsetToStackPointer(instruction);
        ExecuteAddOffsetToStackPointer(instruction);
    }
    else if (PushPopRegisters::IsInstanceOf(instruction))
    {
        if (log) LogPushPopRegisters(instruction);
        ExecutePushPopRegisters(instruction);
    }
    else if (LoadStoreHalfword::IsInstanceOf(instruction))
    {
        if (log) LogLoadStoreHalfword(instruction);
        ExecuteLoadStoreHalfword(instruction);
    }
    else if (SPRelativeLoadStore::IsInstanceOf(instruction))
    {
        if (log) LogSPRelativeLoadStore(instruction);
        ExecuteSPRelativeLoadStore(instruction);
    }
    else if (LoadAddress::IsInstanceOf(instruction))
    {
        if (log) LogLoadAddress(instruction);
        ExecuteLoadAddress(instruction);
    }
    else if (LoadStoreWithImmOffset::IsInstanceOf(instruction))
    {
        if (log) LogLoadStoreWithImmOffset(instruction);
        ExecuteLoadStoreWithImmOffset(instruction);
    }
    else if (LoadStoreWithRegOffset::IsInstanceOf(instruction))
    {
        if (log) LogLoadStoreWithRegOffset(instruction);
        ExecuteLoadStoreWithRegOffset(instruction);
    }
    else if (LoadStoreSignExtendedByteHalfword::IsInstanceOf(instruction))
    {
        if (log) LogLoadStoreSignExtendedByteHalfword(instruction);
        ExecuteLoadStoreSignExtendedByteHalfword(instruction);
    }
    else if (PCRelativeLoad::IsInstanceOf(instruction))
    {
        if (log) LogPCRelativeLoad(instruction);
        ExecutePCRelativeLoad(instruction);
    }
    else if (HiRegisterOperationsBranchExchange::IsInstanceOf(instruction))
    {
        if (log) LogHiRegisterOperationsBranchExchange(instruction);
        ExecuteHiRegisterOperationsBranchExchange(instruction);
    }
    else if (ALUOperations::IsInstanceOf(instruction))
    {
        if (log) LogALUOperations(instruction);
        ExecuteALUOperations(instruction);
    }
    else if (MoveCompareAddSubtractImmediate::IsInstanceOf(instruction))
    {
        if (log) LogMoveCompareAddSubtractImmediate(instruction);
        ExecuteMoveCompareAddSubtractImmediate(instruction);
    }
    else if (AddSubtract::IsInstanceOf(instruction))
    {
        if (log) LogAddSubtract(instruction);
        ExecuteAddSubtract(instruction);
    }
    else if (MoveShiftedRegister::IsInstanceOf(instruction))
    {
        if (log) LogMoveShiftedRegister(instruction);
        ExecuteMoveShiftedRegister(instruction);
    }
    else
    {
        throw std::runtime_error("Unable to decode THUMB instruction");
    }
}

void ARM7TDMI::ExecuteThumbSoftwareInterrupt(u16 instruction)
{
    (void)instruction;
    throw std::runtime_error("ThumbSoftwareInterrupt not implemented");
}

void ARM7TDMI::ExecuteUnconditionalBranch(u16 instruction)
{
    (void)instruction;
    throw std::runtime_error("UnconditionalBranch not implemented");
}

void ARM7TDMI::ExecuteConditionalBranch(u16 instruction)
{
    (void)instruction;
    throw std::runtime_error("ConditionalBranch not implemented");
}

void ARM7TDMI::ExecuteMultipleLoadStore(u16 instruction)
{
    (void)instruction;
    throw std::runtime_error("MultipleLoadStore not implemented");
}

void ARM7TDMI::ExecuteLongBranchWithLink(u16 instruction)
{
    (void)instruction;
    throw std::runtime_error("LongBranchWithLink not implemented");
}

void ARM7TDMI::ExecuteAddOffsetToStackPointer(u16 instruction)
{
    (void)instruction;
    throw std::runtime_error("AddOffsetToStackPointer not implemented");
}

void ARM7TDMI::ExecutePushPopRegisters(u16 instruction)
{
    (void)instruction;
    throw std::runtime_error("PushPopRegisters not implemented");
}

void ARM7TDMI::ExecuteLoadStoreHalfword(u16 instruction)
{
    (void)instruction;
    throw std::runtime_error("LoadStoreHalfword not implemented");
}

void ARM7TDMI::ExecuteSPRelativeLoadStore(u16 instruction)
{
    (void)instruction;
    throw std::runtime_error("SPRelativeLoadStore not implemented");
}

void ARM7TDMI::ExecuteLoadAddress(u16 instruction)
{
    (void)instruction;
    throw std::runtime_error("LoadAddress not implemented");
}

void ARM7TDMI::ExecuteLoadStoreWithImmOffset(u16 instruction)
{
    (void)instruction;
    throw std::runtime_error("LoadStoreWithImmediateOffset not implemented");
}

void ARM7TDMI::ExecuteLoadStoreWithRegOffset(u16 instruction)
{
    (void)instruction;
    throw std::runtime_error("LoadStoreWithRegisterOffset not implemented");
}

void ARM7TDMI::ExecuteLoadStoreSignExtendedByteHalfword(u16 instruction)
{
    (void)instruction;
    throw std::runtime_error("LoadStoreSignExtendedByteHalfword not implemented");
}

void ARM7TDMI::ExecutePCRelativeLoad(u16 instruction)
{
    (void)instruction;
    throw std::runtime_error("PCRelativeLoad not implemented");
}

void ARM7TDMI::ExecuteHiRegisterOperationsBranchExchange(u16 instruction)
{
    (void)instruction;
    throw std::runtime_error("HiRegisterOperationsBranchExchange not implemented");
}

void ARM7TDMI::ExecuteALUOperations(u16 instruction)
{
    (void)instruction;
    throw std::runtime_error("ALUOperations not implemented");
}

void ARM7TDMI::ExecuteMoveCompareAddSubtractImmediate(u16 instruction)
{
    (void)instruction;
    throw std::runtime_error("MoveCompareAddSubtractImmediate not implemented");
}

void ARM7TDMI::ExecuteAddSubtract(u16 instruction)
{
    (void)instruction;
    throw std::runtime_error("AddSubtract not implemented");
}

void ARM7TDMI::ExecuteMoveShiftedRegister(u16 instruction)
{
    (void)instruction;
    throw std::runtime_error("MoveShiftedRegister not implemented");
}
}  // namespace cpu
