#include <GBA/include/CPU/ARM7TDMI.hpp>
#include <bit>
#include <cstring>
#include <stdexcept>
#include <utility>
#include <GBA/include/CPU/THUMB.hpp>
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
/// @param carry Carry flag, defaults to 1 meaning non SBC operation.
/// @return Pair of {carry_flag, overflow_flag}.
std::pair<bool, bool> Sub32(u32 op1, u32 op2, u32& result, bool carry = 1)
{
    u32 carryVal = carry ? 0 : 1;
    carryVal = ~carryVal + 1;
    u64 result64 = static_cast<u64>(op1) + static_cast<u64>(~op2 + 1) + static_cast<u64>(carryVal);
    result = result64 & U32_MAX;
    bool c = op1 >= op2;
    bool v = SubtractionOverflow(op1, op2, result);
    return {c, v};
}

/// @brief Determine the number of internal cycles needed to perform a multiplication op.
/// @param val Current value in destination register.
/// @return Number of internal cycles.
int InternalMultiplyCycles(u32 val)
{
    int cycles;

    if (((val & 0xFFFF'FF00) == 0xFFFF'FF00) || ((val & 0xFFFF'FF00) == 0))
    {
        cycles = 1;
    }
    else if (((val & 0xFFFF'0000) == 0xFFFF'0000) || ((val & 0xFFFF'0000) == 0))
    {
        cycles = 2;
    }
    else if (((val & 0xFF00'0000) == 0xFF00'0000) || ((val & 0xFF00'0000) == 0))
    {
        cycles = 3;
    }
    else
    {
        cycles = 4;
    }

    return cycles;
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
        if (log) LogLoadStoreWithOffset(instruction);
        ExecuteLoadStoreWithOffset(instruction);
    }
    else if (LoadStoreWithRegOffset::IsInstanceOf(instruction))
    {
        if (log) LogLoadStoreWithOffset(instruction);
        ExecuteLoadStoreWithOffset(instruction);
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
    auto flags = std::bit_cast<UnconditionalBranch::Flags>(instruction);
    i16 offset = SignExtend<i16, 11>(flags.Offset11 << 1);
    registers_.SetPC(registers_.GetPC() + offset);
    flushPipeline_ = true;
}

void ARM7TDMI::ExecuteConditionalBranch(u16 instruction)
{
    auto flags = std::bit_cast<ConditionalBranch::Flags>(instruction);

    if (!ConditionSatisfied(flags.Cond))
    {
        return;
    }

    i16 offset = flags.SOffset8 << 1;
    offset = SignExtend<i16, 8>(offset);
    u32 pc = registers_.GetPC() + offset;
    registers_.SetPC(pc);
    flushPipeline_ = true;
}

void ARM7TDMI::ExecuteMultipleLoadStore(u16 instruction)
{
    auto flags = std::bit_cast<MultipleLoadStore::Flags>(instruction);
    u8 regList = flags.Rlist;
    u32 addr = registers_.ReadRegister(flags.Rb);
    u32 wbAddr = addr;
    bool emptyRlist = regList == 0;
    bool rbInList = regList & (0x01 << flags.Rb);
    bool rbFirstInList = rbInList && !flags.L && ((((0x01 << flags.Rb) - 1) & regList) == 0);

    if (!rbFirstInList)
    {
        wbAddr += (4 * std::popcount(regList));
    }

    if (flags.L)
    {
        u8 regIndex = 0;

        while (regList != 0)
        {
            if (regList & 0x01)
            {
                auto [val, readCycles] = ReadMemory(addr, AccessSize::WORD);
                scheduler_.Step(readCycles);
                registers_.WriteRegister(regIndex, val);
                addr += 4;
            }

            ++regIndex;
            regList >>= 1;
        }

        if (emptyRlist)
        {
            auto [val, readCycles] = ReadMemory(addr, AccessSize::WORD);
            scheduler_.Step(readCycles);
            registers_.SetPC(val);
            flushPipeline_ = true;
        }
    }
    else
    {
        u8 regIndex = 0;

        while (regList != 0)
        {
            if (regList & 0x01)
            {
                u32 val = registers_.ReadRegister(regIndex);

                if (!rbFirstInList && (regIndex == flags.Rb))
                {
                    val = wbAddr;
                }

                int writeCycles = WriteMemory(addr, val, AccessSize::WORD);
                scheduler_.Step(writeCycles);
                addr += 4;
            }

            ++regIndex;
            regList >>= 1;
        }

        if (emptyRlist)
        {
            u32 value = registers_.GetPC() + 2;
            int writeCycles = WriteMemory(addr, value, AccessSize::WORD);
            scheduler_.Step(writeCycles);
        }
    }

    if (emptyRlist)
    {
        wbAddr = registers_.ReadRegister(flags.Rb) + 0x40;
        registers_.WriteRegister(flags.Rb, wbAddr);
    }
    else if (!(rbInList && flags.L))
    {
        registers_.WriteRegister(flags.Rb, addr);
    }

    if (flags.L)
    {
        scheduler_.Step(1);
    }
}

void ARM7TDMI::ExecuteLongBranchWithLink(u16 instruction)
{
    auto flags = std::bit_cast<LongBranchWithLink::Flags>(instruction);

    if (!flags.H)
    {
        // Instruction 1
        i32 offset = flags.Offset << 12;
        offset = SignExtend<i32, 22>(offset);
        u32 lr = registers_.GetPC() + offset;
        registers_.WriteRegister(LR_INDEX, lr);
    }
    else
    {
        // Instruction 2
        u32 offset = flags.Offset << 1;
        u32 lr = (registers_.GetPC() - 2) | 0x01;
        u32 pc = registers_.ReadRegister(LR_INDEX) + offset;
        registers_.WriteRegister(LR_INDEX, lr);
        registers_.SetPC(pc);
        flushPipeline_ = true;
    }
}

void ARM7TDMI::ExecuteAddOffsetToStackPointer(u16 instruction)
{
    auto flags = std::bit_cast<AddOffsetToStackPointer::Flags>(instruction);
    u16 offset = flags.SWord7 << 2;
    u32 sp = registers_.ReadRegister(SP_INDEX);
    sp += flags.S ? -offset : offset;
    registers_.WriteRegister(SP_INDEX, sp);
}

void ARM7TDMI::ExecutePushPopRegisters(u16 instruction)
{
    auto flags = std::bit_cast<PushPopRegisters::Flags>(instruction);
    u8 regList = flags.Rlist;
    bool emptyRlist = (regList == 0) && !flags.R;
    u32 addr = registers_.ReadRegister(SP_INDEX);

    if (flags.L)
    {
        // POP
        u8 regIndex = 0;

        while (regList != 0)
        {
            if (regList & 0x01)
            {
                auto [val, readCycles] = ReadMemory(addr, AccessSize::WORD);
                scheduler_.Step(readCycles);
                registers_.WriteRegister(regIndex, val);
                addr += 4;
            }

            ++regIndex;
            regList >>= 1;
        }

        if (flags.R || emptyRlist)
        {
            auto [val, readCycles] = ReadMemory(addr, AccessSize::WORD);
            scheduler_.Step(readCycles);
            registers_.SetPC(val);
            addr += 4;
            flushPipeline_ = true;
        }
    }
    else
    {
        // PUSH
        if (flags.R)
        {
            addr -= 4;
            u32 val = registers_.ReadRegister(LR_INDEX);
            int writeCycles = WriteMemory(addr, val, AccessSize::WORD);
            scheduler_.Step(writeCycles);
        }
        else if (emptyRlist)
        {
            addr -= 4;
            u32 val = registers_.GetPC() + 2;
            int writeCycles = WriteMemory(addr, val, AccessSize::WORD);
            scheduler_.Step(writeCycles);
        }

        u8 regIndex = 7;

        while (regList != 0)
        {
            if (regList & 0x80)
            {
                addr -= 4;
                u32 value = registers_.ReadRegister(regIndex);
                int writeCycles = WriteMemory(addr, value, AccessSize::WORD);
                scheduler_.Step(writeCycles);
            }

            --regIndex;
            regList <<= 1;
        }
    }

    if (emptyRlist)
    {
        u32 sp = registers_.ReadRegister(SP_INDEX) + (flags.L ? 0x40 : -0x40);
        registers_.WriteRegister(SP_INDEX, sp);
    }
    else
    {
        registers_.WriteRegister(SP_INDEX, addr);
    }

    if (flags.L)
    {
        scheduler_.Step(1);
    }
}

void ARM7TDMI::ExecuteLoadStoreHalfword(u16 instruction)
{
    auto flags = std::bit_cast<LoadStoreHalfword::Flags>(instruction);
    u32 addr = registers_.ReadRegister(flags.Rb) + (flags.Offset5 << 1);

    if (flags.L)
    {
        auto [val, readCycles] = ReadMemory(addr, AccessSize::HALFWORD);
        scheduler_.Step(readCycles);

        if (addr & 0x01)
        {
            val = std::rotr(val, 8);
        }

        registers_.WriteRegister(flags.Rd, val);
        scheduler_.Step(1);
    }
    else
    {
        u16 val = registers_.ReadRegister(flags.Rd);
        int writeCycles = WriteMemory(addr, val, AccessSize::HALFWORD);
        scheduler_.Step(writeCycles);
    }
}

void ARM7TDMI::ExecuteSPRelativeLoadStore(u16 instruction)
{
    auto flags = std::bit_cast<SPRelativeLoadStore::Flags>(instruction);
    u32 addr = registers_.ReadRegister(SP_INDEX) + (flags.Word8 << 2);

    if (flags.L)
    {
        auto [val, readCycles] = ReadMemory(addr, AccessSize::WORD);
        scheduler_.Step(readCycles);

        if (addr & 0x03)
        {
            val = std::rotr(val, (addr & 0x03) * 8);
        }

        registers_.WriteRegister(flags.Rd, val);
        scheduler_.Step(1);
    }
    else
    {
        u32 val = registers_.ReadRegister(flags.Rd);
        int writeCycles = WriteMemory(addr, val, AccessSize::WORD);
        scheduler_.Step(writeCycles);
    }
}

void ARM7TDMI::ExecuteLoadAddress(u16 instruction)
{
    auto flags = std::bit_cast<LoadAddress::Flags>(instruction);
    u16 offset = flags.Word8 << 2;
    u32 addr;

    if (flags.SP)
    {
        addr = registers_.ReadRegister(SP_INDEX);
    }
    else
    {
        addr = registers_.GetPC() & 0xFFFF'FFFD;
    }

    addr += offset;
    registers_.WriteRegister(flags.Rd, addr);
}

void ARM7TDMI::ExecuteLoadStoreWithOffset(u16 instruction)
{
    u32 addr;
    u8 Rd;
    AccessSize length;
    bool load;

    if (LoadStoreWithImmOffset::IsInstanceOf(instruction))
    {
        auto flags = std::bit_cast<LoadStoreWithImmOffset::Flags>(instruction);
        u8 offset = flags.B ? flags.Offset5 : (flags.Offset5 << 2);
        addr = registers_.ReadRegister(flags.Rb) + offset;
        length = flags.B ? AccessSize::BYTE : AccessSize::WORD;
        Rd = flags.Rd;
        load = flags.L;
    }
    else
    {
        auto flags = std::bit_cast<LoadStoreWithRegOffset::Flags>(instruction);
        addr = registers_.ReadRegister(flags.Rb) + registers_.ReadRegister(flags.Ro);
        length = flags.B ? AccessSize::BYTE : AccessSize::WORD;
        Rd = flags.Rd;
        load = flags.L;
    }

    if (load)
    {
        auto [val, readCycles] = ReadMemory(addr, length);
        scheduler_.Step(readCycles);

        if ((length == AccessSize::WORD) && (addr & 0x03))
        {
            val = std::rotr(val, (addr & 0x03) * 8);
        }

        registers_.WriteRegister(Rd, val);
        scheduler_.Step(1);
    }
    else
    {
        u32 val = registers_.ReadRegister(Rd);
        int writeCycles = WriteMemory(addr, val, length);
        scheduler_.Step(writeCycles);
    }
}

void ARM7TDMI::ExecuteLoadStoreSignExtendedByteHalfword(u16 instruction)
{
    auto flags = std::bit_cast<LoadStoreSignExtendedByteHalfword::Flags>(instruction);
    u32 addr = registers_.ReadRegister(flags.Rb) + registers_.ReadRegister(flags.Ro);
    bool load = true;
    bool h = flags.H;

    if (flags.S)
    {
        i32 val;
        int readCycles;

        if (h && (addr & 0x01))
        {
            // Convert LDRSH into LDRSB
            h = false;
        }

        if (h)
        {
            // LDSH
            std::tie(val, readCycles) = ReadMemory(addr, AccessSize::HALFWORD);
            val = SignExtend<i32, 15>(val);
        }
        else
        {
            // LDSB
            std::tie(val, readCycles) = ReadMemory(addr, AccessSize::BYTE);
            val = SignExtend<i32, 7>(val);
        }

        scheduler_.Step(readCycles);
        registers_.WriteRegister(flags.Rd, val);
    }
    else
    {
        if (h)
        {
            // LDRH
            auto [val, readCycles] = ReadMemory(addr, AccessSize::HALFWORD);
            scheduler_.Step(readCycles);

            if (addr & 0x01)
            {
                val = std::rotr(val, 8);
            }

            registers_.WriteRegister(flags.Rd, val);
        }
        else
        {
            // STRH
            load = false;
            u32 val = registers_.ReadRegister(flags.Rd);
            int writeCycles = WriteMemory(addr, val, AccessSize::HALFWORD);
            scheduler_.Step(writeCycles);
        }
    }

    if (load)
    {
        scheduler_.Step(1);
    }
}

void ARM7TDMI::ExecutePCRelativeLoad(u16 instruction)
{
    auto flags = std::bit_cast<PCRelativeLoad::Flags>(instruction);
    u32 addr = (registers_.GetPC() & 0xFFFF'FFFC) + (flags.Word8 << 2);
    auto [val, readCycles] = ReadMemory(addr, AccessSize::WORD);
    scheduler_.Step(readCycles);

    if (addr & 0x03)
    {
        val = std::rotr(val, (addr & 0x03) * 8);
    }

    registers_.WriteRegister(flags.Rd, val);
}

void ARM7TDMI::ExecuteHiRegisterOperationsBranchExchange(u16 instruction)
{
    auto flags = std::bit_cast<HiRegisterOperationsBranchExchange::Flags>(instruction);
    u8 Rd = flags.RdHd;
    u8 Rs = flags.RsHs;

    if (flags.H1)
    {
        Rd += 8;
    }

    if (flags.H2)
    {
        Rs += 8;
    }

    switch (flags.Op)
    {
        case 0b00:  // ADD
        {
            u32 result = registers_.ReadRegister(Rd) + registers_.ReadRegister(Rs);
            registers_.WriteRegister(Rd, result);
            flushPipeline_ = Rd == PC_INDEX;
            break;
        }
        case 0b01:  // CMP
        {
            u32 op1 = registers_.ReadRegister(Rd);
            u32 op2 = registers_.ReadRegister(Rs);
            u32 result = 0;

            auto [carry, overflow] = Sub32(op1, op2, result);

            registers_.SetNegative(result & U32_MSB);
            registers_.SetZero(result == 0);
            registers_.SetCarry(carry);
            registers_.SetOverflow(overflow);
            break;
        }
        case 0b10:  // MOV
            registers_.WriteRegister(Rd, registers_.ReadRegister(Rs));
            flushPipeline_ = Rd == PC_INDEX;
            break;
        case 0b11:  // BX
        {
            u32 pc = registers_.ReadRegister(Rs);
            flushPipeline_ = true;

            if (pc & 0x01)
            {
                registers_.SetOperatingState(OperatingState::THUMB);
            }
            else
            {
                registers_.SetOperatingState(OperatingState::ARM);
            }

            registers_.SetPC(pc);
            break;
        }
    }
}

void ARM7TDMI::ExecuteALUOperations(u16 instruction)
{
    auto flags = std::bit_cast<ALUOperations::Flags>(instruction);
    bool saveResult = true;
    bool updateCarry = true;
    bool updateOverflow = true;
    bool carry = registers_.IsCarry();
    bool overflow = registers_.IsOverflow();

    u32 op1 = registers_.ReadRegister(flags.Rd);
    u32 op2 = registers_.ReadRegister(flags.Rs);
    u32 result;

    switch (flags.Op)
    {
        case 0b0000:  // AND
            result = op1 & op2;
            updateCarry = false;
            updateOverflow = false;
            break;
        case 0b0001:  // EOR
            result = op1 ^ op2;
            updateCarry = false;
            updateOverflow = false;
            break;
        case 0b0010:  // LSL
        {
            op2 &= 0xFF;
            result = op1;
            updateOverflow = false;

            if (op2 > 32)
            {
                carry = false;
                result = 0;
            }
            else if (op2 == 32)
            {
                carry = (op1 & 0x01);
                result = 0;
            }
            else if (op2 != 0)
            {
                carry = op1 & (U32_MSB >> (op2 - 1));
                result <<= op2;
            }

            scheduler_.Step(1);
            break;
        }
        case 0b0011:  // LSR
        {
            op2 &= U8_MAX;
            result = op1;
            updateOverflow = false;

            if (op2 > 32)
            {
                carry = false;
                result = 0;
            }
            else if (op2 == 32)
            {
                carry = op1 & U32_MSB;
                result = 0;
            }
            else if (op2 != 0)
            {
                carry = op1 & (0x01 << (op2 - 1));
                result >>= op2;
            }

            scheduler_.Step(1);
            break;
        }
        case 0b0100:  // ASR
        {
            op2 &= U8_MAX;
            result = op1;
            updateOverflow = false;
            bool msbSet = op1 & U32_MSB;

            if (op2 >= 32)
            {
                carry = msbSet;
                result = msbSet ? U32_MAX : 0;
            }
            else if (op2 > 0)
            {
                carry = op1 & (0x01 << (op2 - 1));

                while (op2 > 0)
                {
                    result >>= 1;
                    result |= (msbSet ? U32_MSB : 0);
                    --op2;
                }
            }

            scheduler_.Step(1);
            break;
        }
        case 0b0101:  // ADC
            std::tie(carry, overflow) = Add32(op1, op2, result, carry);
            break;
        case 0b0110:  // SBC
            std::tie(carry, overflow) = Sub32(op1, op2, result, carry);
            break;
        case 0b0111:  // ROR
        {
            op2 &= U8_MAX;
            result = op1;
            updateOverflow = false;

            if (op2 > 32)
            {
                op2 %= 32;
            }

            if (op2)
            {
                carry = op1 & (0x01 << (op2 - 1));
                result = std::rotr(result, op2);
            }

            scheduler_.Step(1);
            break;
        }
        case 0b1000:  // TST
            result = op1 & op2;
            saveResult = false;
            updateCarry = false;
            updateOverflow = false;
            break;
        case 0b1001:  // NEG
            std::tie(carry, overflow) = Sub32(0, op2, result);
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
            updateCarry = false;
            updateOverflow = false;
            break;
        case 0b1101:  // MUL
            result = op1 * op2;
            updateOverflow = false;
            scheduler_.Step(InternalMultiplyCycles(op1));
            break;
        case 0b1110:  // BIC
            result = op1 & ~op2;
            updateCarry = false;
            updateOverflow = false;
            break;
        case 0b1111:  // MVN
            result = ~op2;
            updateCarry = false;
            updateOverflow = false;
            break;
    }

    registers_.SetNegative(result & U32_MSB);
    registers_.SetZero(result == 0);

    if (updateCarry)
    {
        registers_.SetCarry(carry);
    }

    if (updateOverflow)
    {
        registers_.SetOverflow(overflow);
    }

    if (saveResult)
    {
        registers_.WriteRegister(flags.Rd, result);
    }
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
    auto flags = std::bit_cast<AddSubtract::Flags>(instruction);
    u32 op1 = registers_.ReadRegister(flags.Rs);
    u32 op2 = flags.I ? flags.RnOffset3 : registers_.ReadRegister(flags.RnOffset3);
    bool carry;
    bool overflow;
    u32 result;

    if (flags.Op)
    {
        std::tie(carry, overflow) = Sub32(op1, op2, result);
    }
    else
    {
        std::tie(carry, overflow) = Add32(op1, op2, result);
    }

    registers_.SetNegative(result & U32_MSB);
    registers_.SetZero(result == 0);
    registers_.SetCarry(carry);
    registers_.SetOverflow(overflow);
    registers_.WriteRegister(flags.Rd, result);
}

void ARM7TDMI::ExecuteMoveShiftedRegister(u16 instruction)
{
    auto flags = std::bit_cast<MoveShiftedRegister::Flags>(instruction);
    bool carry = registers_.IsCarry();
    u8 shiftAmount = flags.Offset5;
    u32 result = registers_.ReadRegister(flags.Rs);

    switch (flags.Op)
    {
        case 0b00:  // LSL
            carry = (shiftAmount == 0) ? carry : (result & (U32_MSB >> (shiftAmount - 1)));
            result <<= shiftAmount;
            break;
        case 0b01:
            carry = (shiftAmount == 0) ? (result & U32_MSB) : (result & (0x01 << (shiftAmount - 1)));
            result = (shiftAmount == 0) ? 0 : (result >> shiftAmount);
            break;
        case 0b10:
        {
            bool msbSet = result & U32_MSB;

            if (shiftAmount == 0)
            {
                carry = msbSet;
                result = msbSet ? U32_MAX : 0;
            }
            else
            {
                carry = result & (0x01 << (shiftAmount - 1));

                for (u8 i = 0; i < shiftAmount; ++i)
                {
                    result >>= 1;
                    result |= (msbSet ? U32_MSB : 0);
                }
            }

            break;
        }
        default:
            break;
    }

    registers_.SetNegative(result & U32_MSB);
    registers_.SetZero(result == 0);
    registers_.SetCarry(carry);
    registers_.WriteRegister(flags.Rd, result);
}
}  // namespace cpu
