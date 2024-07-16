#include <GBA/include/CPU/ARM7TDMI.hpp>
#include <bit>
#include <cstring>
#include <format>
#include <sstream>
#include <string>
#include <GBA/include/CPU/ARM.hpp>
#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/Types.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>

namespace
{
/// @brief Convert ARM condition code to its mnemonic.
/// @param condition 4-bit ARM condition code.
/// @return Condition mnemonic.
std::string ConditionMnemonic(u8 condition)
{
    switch (condition)
    {
        case 0:
            return "EQ";
        case 1:
            return "NE";
        case 2:
            return "CS";
        case 3:
            return "CC";
        case 4:
            return "MI";
        case 5:
            return "PL";
        case 6:
            return "VS";
        case 7:
            return "VC";
        case 8:
            return "HI";
        case 9:
            return "LS";
        case 10:
            return "GE";
        case 11:
            return "LT";
        case 12:
            return "GT";
        case 13:
            return "LE";
        case 14:
            return "";
        default:
            break;
    }

    return "";
}

/// @brief Help write LDM/STM formatted register string.
/// @param regStream Stringstream to write to.
/// @param consecutiveRegisters How many consecutive registers were included in the operation.
/// @param regIndex Current index of register not included in operation.
void BlockDataTransferHelper(std::stringstream& regStream, int consecutiveRegisters, u8 regIndex)
{
    if (consecutiveRegisters <= 2)
    {
        for (int r = regIndex - consecutiveRegisters; r < regIndex; ++r)
        {
            if (r == cpu::LR_INDEX)
            {
                regStream << "LR, ";
            }
            else if (r == cpu::PC_INDEX)
            {
                regStream << "PC, ";
            }
            else
            {
                regStream << "R" << r << ", ";
            }
        }
    }
    else
    {
        regStream << "R" << (regIndex - consecutiveRegisters) << "-R" << (regIndex - 1) << ", ";
    }
}
}

namespace cpu
{
using namespace arm;

void ARM7TDMI::LogBranchAndExchange(u32 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogBlockDataTransfer(u32 instruction) const
{
    auto flags = std::bit_cast<BlockDataTransfer::Flags>(instruction);
    std::string cond = ConditionMnemonic(flags.Cond);
    std::string op;

    u8 addrReg = flags.Rn;
    bool isStackOp = addrReg == SP_INDEX;
    std::string addr = isStackOp ? "SP" : std::format("R{}", addrReg);

    u8 addressingModeCase =
        (flags.L ? 0b100 : 0) | (flags.P ? 0b010 : 0) | (flags.U ? 0b001 : 0);

    switch (addressingModeCase)
    {
        case 0b000:
            op = isStackOp ? "STMED" : "STMDA";
            break;
        case 0b001:
            op = isStackOp ? "STMEA" : "STMIA";
            break;
        case 0b010:
            op = isStackOp ? "STMFD" : "STMDB";
            break;
        case 0b011:
            op = isStackOp ? "STMFA" : "STMIB";
            break;
        case 0b100:
            op = isStackOp ? "LDMFA" : "LDMDA";
            break;
        case 0b101:
            op = isStackOp ? "LDMFD" : "LDMIA";
            break;
        case 0b110:
            op = isStackOp ? "LDMEA" : "LDMDB";
            break;
        case 0b111:
            op = isStackOp ? "LDMED" : "LDMIB";
            break;
    }

    u8 regIndex = 0;
    std::stringstream regStream;
    u16 regList = flags.RegisterList;
    int consecutiveRegisters = 0;
    regStream << "{";

    while (regList != 0)
    {
        if (regList & 0x01)
        {
            ++consecutiveRegisters;
        }
        else if (consecutiveRegisters > 0)
        {
            BlockDataTransferHelper(regStream, consecutiveRegisters, regIndex);
            consecutiveRegisters = 0;
        }

        ++regIndex;
        regList >>= 1;
    }

    if (consecutiveRegisters > 0)
    {
        BlockDataTransferHelper(regStream, consecutiveRegisters, regIndex);
    }

    if (regStream.str().length() > 1)
    {
        regStream.seekp(-2, regStream.cur);
    }

    regStream << "}";

    std::string mnemonic =
        std::format("{:08X} -> {}{} {}{}, {}{}",
                    instruction, op, cond, addr, flags.W ? "!" : "", regStream.str(), flags.S ? "^" : "");

    log_.LogCPU(mnemonic, registers_.RegistersString(), logPC_);
}

void ARM7TDMI::LogBranch(u32 instruction) const
{
    auto flags = std::bit_cast<Branch::Flags>(instruction);
    std::string cond = ConditionMnemonic(flags.Cond);
    std::string op = flags.L ? "BL" : "B";

    i32 offset = SignExtend<i32, 25>(flags.Offset << 2);
    u32 newPC = registers_.GetPC() + offset;

    std::string mnemonic = std::format("{:08X} -> {}{} 0x{:08X}", instruction, op, cond, newPC);
    log_.LogCPU(mnemonic, registers_.RegistersString(), logPC_);
}

void ARM7TDMI::LogArmSoftwareInterrupt(u32 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogUndefined(u32 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogSingleDataTransfer(u32 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogSingleDataSwap(u32 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogMultiply(u32 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogMultiplyLong(u32 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogHalfwordDataTransferRegOffset(u32 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogHalfwordDataTransferImmOffset(u32 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogPSRTransferMRS(u32 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogPSRTransferMSR(u32 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogDataProcessing(u32 instruction) const
{
    auto flags = std::bit_cast<DataProcessing::Flags>(instruction);
    std::string cond = ConditionMnemonic(flags.Cond);
    std::string op;
    std::string s = flags.S ? "S" : "";

    if ((8 <= flags.OpCode) && (flags.OpCode <= 11))
    {
        s = "";
    }

    switch (flags.OpCode)
    {
        case 0b0000:
            op = "AND";
            break;
        case 0b0001:
            op = "EOR";
            break;
        case 0b0010:
            op = "SUB";
            break;
        case 0b0011:
            op = "RSB";
            break;
        case 0b0100:
            op = "ADD";
            break;
        case 0b0101:
            op = "ADC";
            break;
        case 0b0110:
            op = "SBC";
            break;
        case 0b0111:
            op = "RSC";
            break;
        case 0b1000:
            op = "TST";
            break;
        case 0b1001:
            op = "TEQ";
            break;
        case 0b1010:
            op = "CMP";
            break;
        case 0b1011:
            op = "CMN";
            break;
        case 0b1100:
            op = "ORR";
            break;
        case 0b1101:
            op = "MOV";
            break;
        case 0b1110:
            op = "BIC";
            break;
        case 0b1111:
            op = "MVN";
            break;
    }

    u32 op2;
    std::string op2Str;

    if (flags.I)
    {
        auto op2SrcFlags = std::bit_cast<DataProcessing::RotatedImmSrc>(instruction);
        op2 = op2SrcFlags.Imm;
        u8 rotate = op2SrcFlags.Rotate << 1;
        op2 = std::rotr(op2, rotate);
        op2Str = std::format("#{}", op2);
    }
    else
    {
        u8 shiftType;
        u8 shiftAmount;
        u8 Rm;
        u8 Rs = 0;
        std::string shiftTypeStr;
        bool isRRX = false;

        if (flags.RegShift)
        {
            auto op2SrcFlags = std::bit_cast<DataProcessing::RegShiftedRegSrc>(instruction);
            Rm = op2SrcFlags.Rm;
            op2 = registers_.ReadRegister(Rm);
            shiftType = op2SrcFlags.ShiftOp;
            Rs = op2SrcFlags.Rs;
            shiftAmount = registers_.ReadRegister(Rs) & U8_MAX;

            if (op2SrcFlags.Rm == PC_INDEX)
            {
                op2 += 4;
            }
        }
        else
        {
            auto op2SrcFlags = std::bit_cast<DataProcessing::ImmShiftedRegSrc>(instruction);
            Rm = op2SrcFlags.Rm;
            op2 = registers_.ReadRegister(Rm);
            shiftType = op2SrcFlags.ShiftOp;
            shiftAmount = op2SrcFlags.Imm;
            isRRX = (op2SrcFlags.Imm == 0) && (op2SrcFlags.ShiftOp == 3);
        }

        switch (shiftType)
        {
            case 0b00:  // LSL
            {
                if (shiftAmount >= 32)
                {
                    op2 = 0;
                }
                else if (shiftAmount != 0)
                {
                    op2 <<= shiftAmount;
                }

                shiftTypeStr = "LSL";
                break;
            }
            case 0b01:  // LSR
            {
                if (shiftAmount >= 32)
                {
                    op2 = 0;
                }
                else if (shiftAmount != 0)
                {
                    op2 >>= shiftAmount;
                }
                else if (!flags.RegShift)
                {
                    op2 = 0;
                }

                shiftTypeStr = "LSR";
                shiftAmount = (shiftAmount == 0) ? 32 : shiftAmount;
                break;
            }
            case 0b10:  // ASR
            {
                bool msbSet = op2 & U32_MSB;

                if (shiftAmount >= 32)
                {
                    op2 = msbSet ? U32_MAX : 0;
                }
                else if (shiftAmount != 0)
                {
                    for (u8 i = 0; i < shiftAmount; ++i)
                    {
                        op2 >>= 1;
                        op2 |= msbSet ? U32_MSB : 0;
                    }
                }
                else if (!flags.RegShift)
                {
                    op2 = msbSet ? U32_MAX : 0;
                }

                shiftTypeStr = "ASR";
                shiftAmount = (shiftAmount == 0) ? 32 : shiftAmount;
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
                        op2 >>= 1;
                        op2 |= registers_.IsCarry() ? U32_MSB : 0;
                    }
                }
                else
                {
                    op2 = std::rotr(op2, shiftAmount);
                }

                shiftTypeStr = isRRX ? "RRX" : "ROR";
                break;
            }
        }

        if (flags.RegShift)
        {
            op2Str = std::format("R{}, {} R{}", Rm, shiftTypeStr, Rs);
        }
        else
        {
            if (isRRX)
            {
                op2Str = std::format("R{}, {}", Rm, shiftTypeStr);
            }
            else if ((shiftTypeStr == "LSL") && (shiftAmount == 0))
            {
                op2Str = std::format("R{}", Rm);
            }
            else
            {
                op2Str = std::format("R{}, {} #{}", Rm, shiftTypeStr, shiftAmount);
            }
        }
    }

    std::string regInfo = "";
    u8 destIndex = flags.Rd;
    u8 op1Index = flags.Rn;

    switch (flags.OpCode)
    {
        case 0b1101:  // MOV
        case 0b1111:  // MVN
            regInfo = std::format("R{}, {}", destIndex, op2Str);
            break;
        case 0b1010:  // CMP
        case 0b1011:  // CMN
        case 0b1001:  // TEQ
        case 0b1000:  // TST
            regInfo = std::format("R{}, {}", op1Index, op2Str);
            break;
        case 0b0000:  // AND
        case 0b0001:  // EOR
        case 0b0010:  // SUB
        case 0b0011:  // RSB
        case 0b0100:  // ADD
        case 0b0101:  // ADC
        case 0b0110:  // SBC
        case 0b0111:  // RSC
        case 0b1100:  // ORR
        case 0b1110:  // BIC
            regInfo = std::format("R{}, R{}, {}", destIndex, op1Index, op2Str);
            break;
    }

    std::string mnemonic = std::format("{:08X} -> {}{}{} {}", instruction, op, cond, s, regInfo);
    log_.LogCPU(mnemonic, registers_.RegistersString(), logPC_);
}
}  // namespace cpu
