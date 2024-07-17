#include <GBA/include/CPU/ARM7TDMI.hpp>
#include <bit>
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

/// @brief Form a mnemonic for halfword data transfer ops.
/// @param instruction 32-bit ARM instruction.
/// @param load Whether this is a load or store op.
/// @param cond ARM condition code.
/// @param destIndex Index of destination register.
/// @param baseIndex Index of base register.
/// @param s S flag.
/// @param h H flag.
/// @param up Add or Subtract offset from base.
/// @param preIndexed Pre/Post indexed op.
/// @param writeBack Whether to write back to 
/// @param offsetExpression String of either immediate offset or offset register index.
/// @return Mnemonic of instruction.
std::string HalfwordDataTransferHelper(u32 instruction,
                                       bool load,
                                       u8 cond,
                                       u8 destIndex,
                                       u8 baseIndex,
                                       bool s,
                                       bool h,
                                       bool up,
                                       bool preIndexed,
                                       bool writeBack,
                                       std::string offsetExpression)
{
    std::string op = load ? "LDR" : "STR";
    std::string opType;

    if (s)
    {
        opType = h ? "SH" : "SB";
    }
    else
    {
        opType = "H";
    }

    std::string address;

    if (offsetExpression == "")
    {
        address = std::format("[R{}]", baseIndex);
    }
    else
    {
        if (preIndexed)
        {
            address = std::format("[R{}, {}{}]{}", baseIndex, up ? "+" : "-", offsetExpression, writeBack ? "!" : "");
        }
        else
        {
            address = std::format("[R{}], {}{}", baseIndex, up ? "+" : "-", offsetExpression);
        }
    }

    return std::format("{:08X} -> {}{}{} R{}, {}", instruction, op, opType, ConditionMnemonic(cond), destIndex, address);
}
}

namespace cpu
{
using namespace arm;

void ARM7TDMI::LogBranchAndExchange(u32 instruction) const
{
    auto flags = std::bit_cast<BranchAndExchange::Flags>(instruction);
    u8 Rn = flags.Rn;
    std::string mnemonic = std::format("{:08X} -> BX{} R{}", instruction, ConditionMnemonic(flags.Cond), Rn);
    log_.LogCPU(mnemonic, registers_.RegistersString(), logPC_);
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
    auto flags = std::bit_cast<SoftwareInterrupt::Flags>(instruction);
    std::string cond = ConditionMnemonic(flags.Cond);
    u32 comment = flags.CommentField;
    std::string mnemonic = std::format("{:08X} -> SWI{} #{:06X}", instruction, cond, comment);
    log_.LogCPU(mnemonic, registers_.RegistersString(), logPC_);
}

void ARM7TDMI::LogUndefined(u32 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogSingleDataTransfer(u32 instruction) const
{
    auto flags = std::bit_cast<SingleDataTransfer::Flags>(instruction);
    std::string cond = ConditionMnemonic(flags.Cond);
    std::string op = flags.L ? "LDR" : "STR";
    std::string xfer = flags.B ? "B" : "";
    op = op + cond + xfer;
    std::string address;
    std::string expression;

    u8 Rd = flags.Rd;
    u8 Rn = flags.Rn;

    if (flags.I)
    {
        auto regFlags = std::bit_cast<SingleDataTransfer::RegOffset>(instruction);
        std::string shiftExpression;
        std::string shiftType;
        u8 shiftRegIndex = regFlags.Rm;
        u8 shiftAmount = regFlags.ShiftAmount;

        switch (regFlags.ShiftType)
        {
            case 0b00:
                shiftType = "LSL";
                break;
            case 0b01:
                shiftType = "LSR";
                shiftAmount = (shiftAmount == 0) ? 32 : shiftAmount;
                break;
            case 0b10:
                shiftType = "ASR";
                shiftAmount = (shiftAmount == 0) ? 32 : shiftAmount;
                break;
            case 0b11:
                shiftType = (shiftAmount == 0) ? "RRX" : "ROR";
                break;
        }

        if (shiftType == "RRX")
        {
            shiftExpression = std::format("R{}, RRX", shiftRegIndex);
        }
        else if ((shiftType == "LSL") && (shiftAmount == 0))
        {
            shiftExpression = std::format("R{}", shiftRegIndex);
        }
        else
        {
            shiftExpression = std::format("R{}, {} #{}", shiftRegIndex, shiftType, shiftAmount);
        }

        expression = std::format("{}{}", flags.U ? "+" : "-", shiftExpression);
    }
    else
    {
        auto immFlags = std::bit_cast<SingleDataTransfer::ImmOffset>(instruction);
        u32 offset = immFlags.Imm;

        if (offset == 0)
        {
            expression = "";
        }
        else
        {
            expression = std::format("#{}{}", flags.U ? "+" : "-", offset);
        }
    }

    if (flags.P)
    {
        // Pre-indexed
        if (expression == "")
        {
            address = std::format("[R{}]", Rn);
        }
        else
        {
            address = std::format("[R{}, {}]{}", Rn, expression, flags.W ? "!" : "");
        }
    }
    else
    {
        // Post-indexed
        if (expression == "")
        {
            address = std::format("[R{}]", Rn);
        }
        else
        {
            address = std::format("[R{}], {}", Rn, expression);
        }
    }

    std::string mnemonic = std::format("{:08X} -> {} R{}, {}", instruction, op, Rd, address);
    log_.LogCPU(mnemonic, registers_.RegistersString(), logPC_);
}

void ARM7TDMI::LogSingleDataSwap(u32 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogMultiply(u32 instruction) const
{
    auto flags = std::bit_cast<Multiply::Flags>(instruction);
    std::string cond = ConditionMnemonic(flags.Cond);
    std::string s = flags.S ? "S" : "";
    u8 Rd = flags.Rd;
    u8 Rm = flags.Rm;
    u8 Rs = flags.Rs;
    u8 Rn = flags.Rn;
    std::string mnemonic;

    if (flags.A)
    {
        mnemonic = std::format("{:08X} -> MLA{}{} R{}, R{}, R{}, R{}", instruction, cond, s, Rd, Rm, Rs, Rn);
    }
    else
    {
        mnemonic = std::format("{:08X} -> MUL{}{} R{}, R{}, R{}", instruction, cond, s, Rd, Rm, Rs);
    }

    log_.LogCPU(mnemonic, registers_.RegistersString(), logPC_);
}

void ARM7TDMI::LogMultiplyLong(u32 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogHalfwordDataTransfer(u32 instruction) const
{
    std::string offsetStr;
    bool load;
    u8 cond;
    u8 Rd;
    u8 Rn;
    bool s;
    bool h;
    bool up;
    bool preIndex;
    bool writeBack;

    if (HalfwordDataTransferRegOffset::IsInstanceOf(instruction))
    {
        auto flags = std::bit_cast<HalfwordDataTransferRegOffset::Flags>(instruction);
        u8 Rm = flags.Rm;
        offsetStr = std::format("R{}", Rm);
        cond = flags.Cond;

        Rd = flags.Rd;
        Rn = flags.Rn;
        s = flags.S;
        h = flags.H;
        up = flags.U;
        preIndex = flags.P;
        load = flags.L;
        writeBack = flags.W;
    }
    else
    {
        auto flags = std::bit_cast<HalfwordDataTransferImmOffset::Flags>(instruction);
        u8 offset = (flags.OffsetHi << 4) | flags.OffsetLo;
        offsetStr = (offset == 0) ? "" : std::format("#{}", offset);
        cond = flags.Cond;

        Rd = flags.Rd;
        Rn = flags.Rn;
        s = flags.S;
        h = flags.H;
        up = flags.U;
        preIndex = flags.P;
        load = flags.L;
        writeBack = flags.W;
    }

    std::string mnemonic =
        HalfwordDataTransferHelper(instruction, load, cond, Rd, Rn, s, h, up, preIndex, writeBack, offsetStr);
    log_.LogCPU(mnemonic, registers_.RegistersString(), logPC_);
}

void ARM7TDMI::LogPSRTransferMRS(u32 instruction) const
{
    auto flags = std::bit_cast<PSRTransferMRS::Flags>(instruction);
    std::string cond = ConditionMnemonic(flags.Cond);
    std::string psr = flags.Ps ? "SPSR" : "CPSR";
    u8 Rd = flags.Rd;
    std::string mnemonic = std::format("{:08X} -> MRS{} R{}, {}", instruction, cond, Rd, psr);
    log_.LogCPU(mnemonic, registers_.RegistersString(), logPC_);
}

void ARM7TDMI::LogPSRTransferMSR(u32 instruction) const
{
    auto flags = std::bit_cast<PSRTransferMSR::Flags>(instruction);
    std::string cond = ConditionMnemonic(flags.Cond);

    std::stringstream fields;
    fields << "_";
    fields << (flags.SetFlags ? "f" : "");
    fields << (flags.SetStatus ? "s" : "");
    fields << (flags.SetExtension ? "x" : "");
    fields << (flags.SetControl ? "c" : "");
    std::string fieldsStr = fields.str() == "_fsxc" ? "_all" : fields.str();

    std::string psr = flags.Pd ? "SPSR" : "CPSR";
    psr = psr + fieldsStr;
    std::string expression;

    if (flags.I)
    {
        auto immFlags = std::bit_cast<PSRTransferMSR::ImmSrc>(instruction);
        u32 imm = std::rotr(immFlags.Imm, immFlags.Rotate * 2);
        expression = std::format("{}, #{:08X}", psr, imm);
    }
    else
    {
        auto regFlags = std::bit_cast<PSRTransferMSR::RegSrc>(instruction);
        u8 Rm = regFlags.Rm;
        expression = std::format("{}, R{}", psr, Rm);
    }

    std::string mnemonic = std::format("{:08X} -> MSR{} {}", instruction, cond, expression);
    log_.LogCPU(mnemonic, registers_.RegistersString(), logPC_);
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

    std::string op2Str;

    if (flags.I)
    {
        auto op2SrcFlags = std::bit_cast<DataProcessing::RotatedImmSrc>(instruction);
        u32 op2 = op2SrcFlags.Imm;
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
            shiftType = op2SrcFlags.ShiftOp;
            Rs = op2SrcFlags.Rs;
            shiftAmount = registers_.ReadRegister(Rs) & U8_MAX;
        }
        else
        {
            auto op2SrcFlags = std::bit_cast<DataProcessing::ImmShiftedRegSrc>(instruction);
            Rm = op2SrcFlags.Rm;
            shiftType = op2SrcFlags.ShiftOp;
            shiftAmount = op2SrcFlags.Imm;
            isRRX = (op2SrcFlags.Imm == 0) && (op2SrcFlags.ShiftOp == 3);
        }

        switch (shiftType)
        {
            case 0b00:  // LSL
                shiftTypeStr = "LSL";
                break;
            case 0b01:  // LSR
                shiftTypeStr = "LSR";
                shiftAmount = (shiftAmount == 0) ? 32 : shiftAmount;
                break;
            case 0b10:  // ASR
                shiftTypeStr = "ASR";
                shiftAmount = (shiftAmount == 0) ? 32 : shiftAmount;
                break;
            case 0b11:  // ROR, RRX
            {
                if (shiftAmount > 32)
                {
                    shiftAmount %= 32;
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
