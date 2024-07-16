#include <GBA/include/CPU/ARM7TDMI.hpp>
#include <bit>
#include <cstring>
#include <stdexcept>
#include <utility>
#include <GBA/include/CPU/ARM.hpp>
#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/Types.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>

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
    u64 result64 = static_cast<u64>(op1) + static_cast<u64>(op2) + static_cast<u64>(carry);
    result = result64 & U32_MAX;
    bool c = (result64 > U32_MAX);
    bool v = AdditionOverflow(op1, op2, result);
    return {c, v};
}

/// @brief Calculate the result of a THUMB sub or sub w/ carry operation, along with the carry and overflow flags.
/// @param op1 First subtraction operand.
/// @param op2 Second subtraction operand.
/// @param result Result of subtraction (op1 - op2), returned by reference.
/// @param sbc Whether this is a subtract with carry operation.
/// @param carry Current value of carry flag.
/// @return Pair of {carry_flag, overflow_flag}.
std::pair<bool, bool> Sub32(u32 op1, u32 op2, u32& result, bool sbc = false, bool carry = 0)
{
    u64 result64;
    bool c;

    if (sbc)
    {
        result64 = static_cast<u64>(op1) + static_cast<u64>(~op2) + static_cast<u64>(carry);
        c = (result64 > U32_MAX);
    }
    else
    {
        result64 = static_cast<u64>(op1) + static_cast<u64>(~op2 + 1);
        c = op1 >= op2;
    }

    result = result64 & U32_MAX;
    bool v = SubtractionOverflow(op1, op2, result);
    return {c, v};
}
}

namespace cpu
{
using namespace arm;

void ARM7TDMI::DecodeAndExecuteARM(u32 instruction, bool log)
{
    u8 conditionCode = (instruction & 0xF000'0000) >> 28;
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
    else if (DataProcessing::IsInstanceOf(instruction))
    {
        if (log) LogDataProcessing(instruction);
        if (conditionMet) ExecuteDataProcessing(instruction);
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
    auto flags = std::bit_cast<BlockDataTransfer::Flags>(instruction);

    u16 regList = flags.RegisterList;
    bool emptyRlist = regList == 0;
    bool wbIndexInList = flags.W && (regList & (0x01 << flags.Rn));
    bool wbIndexFirstInList = wbIndexInList && !flags.L && ((((0x01 << flags.Rn) - 1) & regList) == 0);

    OperatingMode mode = registers_.GetOperatingMode();
    bool r15InList = (regList & 0x8000) == 0x8000;

    if (regList == 0)
    {
        regList = 0x8000;
    }

    if (flags.S)
    {
        if (r15InList)
        {
            if (!flags.L)
            {
                mode = OperatingMode::User;
            }
        }
        else
        {
            mode = OperatingMode::User;
        }
    }

    int xferCnt = std::popcount(regList);
    u32 baseAddr = registers_.ReadRegister(flags.Rn);
    u32 minAddr = 0;
    u32 wbAddr = 0;
    bool preIndexOffset = flags.P;

    if (flags.U)
    {
        minAddr = preIndexOffset ? (baseAddr + 4) : baseAddr;

        if (preIndexOffset)
        {
            wbAddr = minAddr + (4 * (xferCnt - 1));
        }
        else
        {
            wbAddr = minAddr + (4 * xferCnt);
        }

        if (emptyRlist)
        {
            wbAddr = baseAddr + 0x40;
        }
    }
    else
    {
        if (preIndexOffset)
        {
            minAddr = baseAddr - (4 * xferCnt);
            wbAddr = minAddr;
        }
        else
        {
            minAddr = baseAddr - (4 * (xferCnt - 1));
            wbAddr = minAddr - 4;
        }

        if (emptyRlist)
        {
            minAddr = baseAddr - (preIndexOffset ? 0x40 : 0x3C);
            wbAddr = baseAddr - 0x40;
        }
    }

    u8 regIndex = 0;
    u32 addr = minAddr;

    while (regList != 0)
    {
        if (regList & 0x01)
        {
            if (flags.L)
            {
                auto [readValue, readCycles] = ReadMemory(addr, AccessSize::WORD);
                scheduler_.Step(readCycles);

                if (regIndex == PC_INDEX)
                {
                    flushPipeline_ = true;

                    if (flags.S)
                    {
                        registers_.LoadSPSR();
                    }
                }

                registers_.WriteRegister(regIndex, readValue, mode);
            }
            else
            {
                u32 regValue = registers_.ReadRegister(regIndex, mode);

                if (regIndex == PC_INDEX)
                {
                    regValue += 4;
                }
                else if ((regIndex == flags.Rn) && !wbIndexFirstInList)
                {
                    regValue = wbAddr;
                }

                int writeCycles = WriteMemory(addr, regValue, AccessSize::WORD);
                scheduler_.Step(writeCycles);
            }

            addr += 4;
        }

        ++regIndex;
        regList >>= 1;
    }

    if (flags.W && !(wbIndexInList && flags.L))
    {
        registers_.WriteRegister(flags.Rn, wbAddr);
    }

    if (flags.L)
    {
        scheduler_.Step(1);
    }
}

void ARM7TDMI::ExecuteBranch(u32 instruction)
{
    auto flags = std::bit_cast<Branch::Flags>(instruction);
    i32 offset = SignExtend<i32, 25>(flags.Offset << 2);

    if (flags.L)
    {
        registers_.WriteRegister(LR_INDEX, (registers_.GetPC() - 4) & 0xFFFF'FFFC);
    }

    registers_.SetPC(registers_.GetPC() + offset);
    flushPipeline_ = true;
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
    auto flags = std::bit_cast<HalfwordDataTransferImmOffset::Flags>(instruction);

    u8 offset = (flags.OffsetHi << 4) | flags.OffsetLo;
    u32 addr = registers_.ReadRegister(flags.Rn);
    bool preIndex = flags.P;
    bool postIndex = !preIndex;
    bool ignoreWriteback = false;

    if (preIndex)
    {
        if (flags.U)
        {
            addr += offset;
        }
        else
        {
            addr -= offset;
        }
    }

    if (flags.L)
    {
        bool s = flags.S;
        bool h = flags.H;
        bool misaligned = addr & 0x01;
        flushPipeline_ = flags.Rd == PC_INDEX;
        ignoreWriteback = flags.Rd == flags.Rn;

        // LDRH Rd,[odd]   -->  LDRH Rd,[odd-1] ROR 8
        // LDRSH Rd,[odd]  -->  LDRSB Rd,[odd]

        if (s)
        {
            i32 val;

            if (misaligned && h)
            {
                // Convert LDRSH into LDRSB
                h = false;
            }

            if (h)
            {
                // S = 1, H = 1
                auto [halfWord, readCycles] = ReadMemory(addr, AccessSize::HALFWORD);
                scheduler_.Step(readCycles);
                val = SignExtend<i32, 15>(halfWord);
            }
            else
            {
                // S = 1, H = 0
                auto [byte, readCycles] = ReadMemory(addr, AccessSize::BYTE);
                scheduler_.Step(readCycles);
                val = SignExtend<i32, 7>(byte);
            }

            registers_.WriteRegister(flags.Rd, val);
        }
        else
        {
            // S = 0, H = 1
            auto [halfWord, readCycles] = ReadMemory(addr, AccessSize::HALFWORD);
            scheduler_.Step(readCycles);

            if (misaligned)
            {
                halfWord = std::rotr(halfWord, 8);
            }

            registers_.WriteRegister(flags.Rd, halfWord);
        }
    }
    else
    {
        // S = 0, H = 1
        u16 halfWord = registers_.ReadRegister(flags.Rd);

        if (flags.Rd == PC_INDEX)
        {
            halfWord += 4;
        }

        int writeCycles = WriteMemory(addr, halfWord, AccessSize::HALFWORD);
        scheduler_.Step(writeCycles);
    }

    if (postIndex)
    {
        if (flags.U)
        {
            addr += offset;
        }
        else
        {
            addr -= offset;
        }
    }

    if (!ignoreWriteback && (flags.W || postIndex))
    {
        registers_.WriteRegister(flags.Rn, addr);
    }

    if (flags.L)
    {
        scheduler_.Step(1);
    }
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
    auto flags = std::bit_cast<DataProcessing::Flags>(instruction);

    u32 op1 = registers_.ReadRegister(flags.Rn);
    u32 op2;

    bool carry = registers_.IsCarry();
    bool overflow = registers_.IsOverflow();

    if (flags.I)
    {
        // Rotated immediate value
        auto op2SrcFlags = std::bit_cast<DataProcessing::RotatedImmSrc>(instruction);
        op2 = op2SrcFlags.Imm;
        u8 rotate = op2SrcFlags.Rotate << 1;
        carry = op2 & (0x01 << (rotate - 1));
        op2 = std::rotr(op2, rotate);
    }
    else
    {
        // Shifted register value
        u8 shiftType;
        u8 shiftAmount;

        if (flags.RegShift)
        {
            // Register shifted by register
            auto op2SrcFlags = std::bit_cast<DataProcessing::RegShiftedRegSrc>(instruction);
            op2 = registers_.ReadRegister(op2SrcFlags.Rm);
            shiftType = op2SrcFlags.ShiftOp;
            shiftAmount = registers_.ReadRegister(op2SrcFlags.Rs) & U8_MAX;

            if (flags.Rn == PC_INDEX)
            {
                op1 += 4;
            }

            if (op2SrcFlags.Rm == PC_INDEX)
            {
                op2 += 4;
            }

            scheduler_.Step(1);
        }
        else
        {
            // Register shifted by immediate
            auto op2SrcFlags = std::bit_cast<DataProcessing::ImmShiftedRegSrc>(instruction);
            op2 = registers_.ReadRegister(op2SrcFlags.Rm);
            shiftType = op2SrcFlags.ShiftOp;
            shiftAmount = op2SrcFlags.Imm;
        }

        switch (shiftType)
        {
            case 0b00:  // LSL
            {
                if (shiftAmount >= 32)
                {
                    carry = (shiftAmount == 32) ? (op2 & 0x01) : false;
                    op2 = 0;
                }
                else if (shiftAmount != 0)
                {
                    carry = op2 & (U32_MSB >> (shiftAmount - 1));
                    op2 <<= shiftAmount;
                }

                break;
            }
            case 0b01:  // LSR
            {
                if (shiftAmount >= 32)
                {
                    carry = (shiftAmount == 32) ? (op2 & U32_MSB) : false;
                    op2 = 0;
                }
                else if (shiftAmount != 0)
                {
                    carry = op2 & (0x01 << (shiftAmount - 1));
                    op2 >>= shiftAmount;
                }
                else if (!flags.RegShift)
                {
                    carry = op2 & U32_MSB;
                    op2 = 0;
                }

                break;
            }
            case 0b10:  // ASR
            {
                bool msbSet = op2 & U32_MSB;

                if (shiftAmount >= 32)
                {
                    carry = msbSet;
                    op2 = msbSet ? U32_MAX : 0;
                }
                else if (shiftAmount != 0)
                {
                    carry = op2 & (0x01 << (shiftAmount - 1));

                    for (u8 i = 0; i < shiftAmount; ++i)
                    {
                        op2 >>= 1;
                        op2 |= msbSet ? U32_MSB : 0;
                    }
                }
                else if (!flags.RegShift)
                {
                    carry = msbSet;
                    op2 = msbSet ? U32_MAX : 0;
                }

                break;
            }
            case 0b11:  // ROR, RRX
            {
                if (shiftAmount > 32)
                {
                    shiftAmount %= 32;
                }

                if (shiftAmount == 0)
                {
                    if (!flags.RegShift)
                    {
                        carry = op2 & 0x01;
                        op2 >>= 1;
                        op2 |= registers_.IsCarry() ? U32_MSB : 0;
                    }
                }
                else
                {
                    carry = op2 & (0x01 << (shiftAmount - 1));
                    op2 = std::rotr(op2, shiftAmount);
                }

                break;
            }
        }
    }

    u32 result;
    bool saveResult = true;
    bool updateOverflowFlag = true;

    switch (flags.OpCode)
    {
        case 0b0000:  // AND
            result = op1 & op2;
            updateOverflowFlag = false;
            break;
        case 0b0001:  // EOR
            result = op1 ^ op2;
            updateOverflowFlag = false;
            break;
        case 0b0010:  // SUB
            std::tie(carry, overflow) = Sub32(op1, op2, result);
            break;
        case 0b0011:  // RSB
            std::tie(carry, overflow) = Sub32(op2, op1, result);
            break;
        case 0b0100:  // ADD
            std::tie(carry, overflow) = Add32(op1, op2, result);
            break;
        case 0b0101:  // ADC
            std::tie(carry, overflow) = Add32(op1, op2, result, registers_.IsCarry());
            break;
        case 0b0110:  // SBC
            std::tie(carry, overflow) = Sub32(op1, op2, result, true, registers_.IsCarry());
            break;
        case 0b0111:  // RSC
            std::tie(carry, overflow) = Sub32(op2, op1, result, true, registers_.IsCarry());
            break;
        case 0b1000:  // TST
            result = op1 & op2;
            updateOverflowFlag = false;
            saveResult = false;
            break;
        case 0b1001:  // TEQ
            result = op1 ^ op2;
            updateOverflowFlag = false;
            saveResult = false;
            break;
        case 0b1010:  // CMP
            std::tie(carry, overflow) = Sub32(op1, op2, result);
            saveResult = false;
            break;
        case 0b1011:  // CMN
            std::tie(carry, overflow) = Add32(op1, op2, result);
            saveResult = false;
            break;
        case 0b1100:  // ORR
            result = op1 | op2;
            updateOverflowFlag = false;
            break;
        case 0b1101:  // MOV
            result = op2;
            updateOverflowFlag = false;
            break;
        case 0b1110:  // BIC
            result = op1 & ~op2;
            updateOverflowFlag = false;
            break;
        case 0b1111:  // MVN
            result = ~op2;
            updateOverflowFlag = false;
            break;
    }

    // destIndex == flags.Rd

    if (flags.S)
    {
        if (flags.Rd == PC_INDEX)
        {
            registers_.LoadSPSR();
            flushPipeline_ = saveResult;
        }
        else
        {
            registers_.SetNegative(result & U32_MSB);
            registers_.SetZero(result == 0);
            registers_.SetCarry(carry);

            if (updateOverflowFlag)
            {
                registers_.SetOverflow(overflow);
            }
        }
    }

    if (saveResult)
    {
        if (!flags.S && (flags.Rd == PC_INDEX))
        {
            flushPipeline_ = true;
        }

        registers_.WriteRegister(flags.Rd, result);
    }
}
}  // namespace cpu::arm
