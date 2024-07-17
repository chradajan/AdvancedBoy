#include <GBA/include/CPU/ARM7TDMI.hpp>
#include <bit>
#include <cstring>
#include <stdexcept>
#include <utility>
#include <GBA/include/CPU/THUMB.hpp>
#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/Types.hpp>

namespace
{
    /// @brief Determine the result of the overflow flag for addition operation: result = op1 + op2
/// @param op1 Addition operand
/// @param op2 Addition operand
/// @param result Addition result
/// @return State of overflow flag after addition
bool AdditionOverflow(u32 op1, u32 op2, u32 result)
{
    return (~(op1 ^ op2) & ((op1 ^ result)) & U32_MSB) != 0;
}

/// @brief Determine the result of the overflow flag for subtraction operation: result = op1 - op2
/// @param op1 Subtraction operand
/// @param op2 Subtraction operand
/// @param result Subtraction result
/// @return State of overflow flag after subtraction
bool SubtractionOverflow(u32 op1, u32 op2, u32 result)
{
    return ((op1 ^ op2) & ((op1 ^ result)) & U32_MSB) != 0;
}

/// @brief Calculate the result of a THUMB add or add w/ carry operation, along with the carry and overflow flags.
/// @param op1 First addition operand.
/// @param op2 Second addition operand.
/// @param result Result of addition, returned by reference.
/// @param carry Carry flag, defaults to 0 meaning non ADC operation.
/// @return Pair of {carry_flag, overflow_flag}.
std::pair<bool, bool> Add32(u32 op1, u32 op2, u32& result, bool carry = 0)
{
    uint64_t result64 = static_cast<uint64_t>(op1) + static_cast<uint64_t>(op2) + static_cast<uint64_t>(carry);
    result = result64 & U32_MAX;
    bool c = (result64 > U32_MAX);
    bool v = AdditionOverflow(op1, op2, result);
    return {c, v};
}

/// @brief Calculate the result of a THUMB sub or sub w/ carry operation, along with the carry and overflow flags.
/// @param op1 First subtraction operand.
/// @param op2 Second subtraction operand.
/// @param result Result of subtraction (op1 - op2), returned by reference.
/// @param carry Carry flag, defaults to 1 meaning non SBC operation.
/// @return Pair of {carry_flag, overflow_flag}.
std::pair<bool, bool> Sub32(u32 op1, u32 op2, u32& result, bool carry = 1)
{
    u32 carryVal = carry ? 0 : 1;
    carryVal = ~carryVal + 1;
    uint64_t result64 = static_cast<uint64_t>(op1) + static_cast<uint64_t>(~op2 + 1) + static_cast<uint64_t>(carryVal);
    result = result64 & U32_MAX;
    bool c = op1 >= op2;
    bool v = SubtractionOverflow(op1, op2, result);
    return {c, v};
}
}

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
    auto flags = std::bit_cast<MoveCompareAddSubtractImmediate::Flags>(instruction);

    bool carry = registers_.IsCarry();
    bool overflow = registers_.IsOverflow();
    bool saveResult = true;
    bool updateAllFlags = true;

    u32 op1 = registers_.ReadRegister(flags.Rd);
    u32 op2 = flags.Offset8;
    u32 result;

    switch (flags.Op)
    {
        case 0b00:  // MOV
            result = op2;
            updateAllFlags = false;
            break;
        case 0b01:  // CMP
            std::tie(carry, overflow) = Sub32(op1, op2, result);
            saveResult = false;
            break;
        case 0b10:  // ADD
            std::tie(carry, overflow) = Add32(op1, op2, result);
            break;
        case 0b11:  // SUB
            std::tie(carry, overflow) = Sub32(op1, op2, result);
            break;
    }

    registers_.SetNegative(result & U32_MSB);
    registers_.SetZero(result == 0);

    if (updateAllFlags)
    {
        registers_.SetCarry(carry);
        registers_.SetOverflow(overflow);
    }

    if (saveResult)
    {
        registers_.WriteRegister(flags.Rd, result);
    }
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
