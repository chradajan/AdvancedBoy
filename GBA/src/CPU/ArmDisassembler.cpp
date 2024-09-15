#include <GBA/include/CPU/ArmDisassembler.hpp>
#include <bit>
#include <format>
#include <sstream>
#include <string>
#include <utility>
#include <GBA/include/CPU/ARM.hpp>
#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/Types/DebugTypes.hpp>
#include <GBA/include/Types/Types.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>

namespace
{
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
/// @return Tuple of strings: {op, cond, args}.
std::tuple<std::string, std::string, std::string> HalfwordDataTransferHelper(bool load,
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

    return {op + opType, cpu::arm::DecodeCondition(cond), std::format("R{}, {}", destIndex, address)};
}
}

namespace cpu::arm
{
using namespace debug::cpu;

Mnemonic DisassembleInstruction(u32 instruction)
{
    Mnemonic mnemonic;
    mnemonic.branchOffset = {};

    if (BranchAndExchange::IsInstanceOf(instruction))
    {
        DisassembleBranchAndExchange(instruction, mnemonic);
    }
    else if (BlockDataTransfer::IsInstanceOf(instruction))
    {
        DisassembleBlockDataTransfer(instruction, mnemonic);
    }
    else if (Branch::IsInstanceOf(instruction))
    {
        DisassembleBranch(instruction, mnemonic);
    }
    else if (SoftwareInterrupt::IsInstanceOf(instruction))
    {
        DisassembleSoftwareInterrupt(instruction, mnemonic);
    }
    else if (Undefined::IsInstanceOf(instruction))
    {
        DisassembleUndefined(instruction, mnemonic);
    }
    else if (SingleDataTransfer::IsInstanceOf(instruction))
    {
        DisassembleSingleDataTransfer(instruction, mnemonic);
    }
    else if (SingleDataSwap::IsInstanceOf(instruction))
    {
        DisassembleSingleDataSwap(instruction, mnemonic);
    }
    else if (Multiply::IsInstanceOf(instruction))
    {
        DisassembleMultiply(instruction, mnemonic);
    }
    else if (MultiplyLong::IsInstanceOf(instruction))
    {
        DisassembleMultiplyLong(instruction, mnemonic);
    }
    else if (HalfwordDataTransferRegOffset::IsInstanceOf(instruction))
    {
        DisassembleHalfwordDataTransfer(instruction, mnemonic);
    }
    else if (HalfwordDataTransferImmOffset::IsInstanceOf(instruction))
    {
        DisassembleHalfwordDataTransfer(instruction, mnemonic);
    }
    else if (PSRTransferMRS::IsInstanceOf(instruction))
    {
        DisassemblePSRTransferMRS(instruction, mnemonic);
    }
    else if (PSRTransferMSR::IsInstanceOf(instruction))
    {
        DisassemblePSRTransferMSR(instruction, mnemonic);
    }
    else if (DataProcessing::IsInstanceOf(instruction))
    {
        DisassembleDataProcessing(instruction, mnemonic);
    }
    else
    {
        mnemonic.op = "???";
        mnemonic.cond = "";
        mnemonic.args = "???";
    }

    return mnemonic;
}

std::string DecodeCondition(u8 cond)
{
    switch (cond)
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

void DisassembleBranchAndExchange(u32 instruction, Mnemonic& mnemonic)
{
    auto flags = std::bit_cast<BranchAndExchange::Flags>(instruction);
    mnemonic.cond = DecodeCondition(flags.Cond);
    mnemonic.op = "BX";
    mnemonic.args = "R" + std::to_string(flags.Rn);
}

void DisassembleBlockDataTransfer(u32 instruction, Mnemonic& mnemonic)
{
    auto flags = std::bit_cast<BlockDataTransfer::Flags>(instruction);
    mnemonic.cond = DecodeCondition(flags.Cond);

    bool isStackOp = flags.Rn == SP_INDEX;
    std::string addr = isStackOp ? "SP" : ("R" + std::to_string(flags.Rn));

    u8 addressingModeCase =
        (flags.L ? 0b100 : 0) | (flags.P ? 0b010 : 0) | (flags.U ? 0b001 : 0);

    switch (addressingModeCase)
    {
        case 0b000:
            mnemonic.op = isStackOp ? "STMED" : "STMDA";
            break;
        case 0b001:
            mnemonic.op = isStackOp ? "STMEA" : "STMIA";
            break;
        case 0b010:
            mnemonic.op = isStackOp ? "STMFD" : "STMDB";
            break;
        case 0b011:
            mnemonic.op = isStackOp ? "STMFA" : "STMIB";
            break;
        case 0b100:
            mnemonic.op = isStackOp ? "LDMFA" : "LDMDA";
            break;
        case 0b101:
            mnemonic.op = isStackOp ? "LDMFD" : "LDMIA";
            break;
        case 0b110:
            mnemonic.op = isStackOp ? "LDMEA" : "LDMDB";
            break;
        case 0b111:
            mnemonic.op = isStackOp ? "LDMED" : "LDMIB";
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
    mnemonic.args = std::format("{}{}, {}{}", addr, flags.W ? "!" : "", regStream.str(), flags.S ? "^" : "");
}

void DisassembleBranch(u32 instruction, Mnemonic& mnemonic)
{
    auto flags = std::bit_cast<Branch::Flags>(instruction);
    mnemonic.cond = DecodeCondition(flags.Cond);
    mnemonic.op = flags.L ? "BL" : "B";

    i32 offset = SignExtend<i32, 25>(flags.Offset << 2);
    offset += 8;
    mnemonic.branchOffset = offset;
    mnemonic.args = "#" + std::to_string(offset);
}

void DisassembleSoftwareInterrupt(u32 instruction, Mnemonic& mnemonic)
{
    auto flags = std::bit_cast<SoftwareInterrupt::Flags>(instruction);
    mnemonic.cond = DecodeCondition(flags.Cond);
    mnemonic.op = "SWI";
    u32 comment = flags.CommentField;
    mnemonic.args = std::format("#{:06X}", comment);
}

void DisassembleUndefined(u32 instruction, Mnemonic& mnemonic)
{
    auto flags = std::bit_cast<Undefined::Flags>(instruction);
    mnemonic.cond = DecodeCondition(flags.Cond);
    mnemonic.op = "UNDEF";
    mnemonic.args = "";
}

void DisassembleSingleDataTransfer(u32 instruction, Mnemonic& mnemonic)
{
    auto flags = std::bit_cast<SingleDataTransfer::Flags>(instruction);
    mnemonic.cond = DecodeCondition(flags.Cond);
    mnemonic.op = flags.L ? "LDR" : "STR";
    mnemonic.op += flags.B ? "B" : "";

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

    mnemonic.args = std::format("R{}, {}", Rd, address);
}

void DisassembleSingleDataSwap(u32 instruction, Mnemonic& mnemonic)
{
    auto flags = std::bit_cast<SingleDataSwap::Flags>(instruction);
    mnemonic.cond = DecodeCondition(flags.Cond);
    mnemonic.op = "SWP";
    mnemonic.op += flags.B ? "B" : "";

    u8 Rd = flags.Rd;
    u8 Rm = flags.Rm;
    u8 Rn = flags.Rn;
    mnemonic.args = std::format("R{}, R{}, [R{}]", Rd, Rm, Rn);
}

void DisassembleMultiply(u32 instruction, Mnemonic& mnemonic)
{
    auto flags = std::bit_cast<Multiply::Flags>(instruction);
    mnemonic.cond = DecodeCondition(flags.Cond);
    mnemonic.op = flags.A ? "MLA" : "MUL";
    mnemonic.op += flags.S ? "S" : "";

    u8 Rd = flags.Rd;
    u8 Rm = flags.Rm;
    u8 Rs = flags.Rs;
    u8 Rn = flags.Rn;

    if (flags.A)
    {
        mnemonic.args = std::format("R{}, R{}, R{}, R{}", Rd, Rm, Rs, Rn);
    }
    else
    {
        mnemonic.args = std::format("R{}, R{}, R{}", Rd, Rm, Rs);
    }
}

void DisassembleMultiplyLong(u32 instruction, Mnemonic& mnemonic)
{
    auto flags = std::bit_cast<MultiplyLong::Flags>(instruction);
    mnemonic.cond = DecodeCondition(flags.Cond);
    mnemonic.op = std::format("{}{}{}", flags.U ? "S" : "U", flags.A ? "MLAL" : "MULL", flags.S ? "S" : "");

    u8 RdHi = flags.RdHi;
    u8 RdLo = flags.RdLo;
    u8 Rs = flags.Rs;
    u8 Rm = flags.Rm;
    mnemonic.args = std::format("R{}, R{}, R{}, R{}", RdLo, RdHi, Rm, Rs);
}

void DisassembleHalfwordDataTransfer(u32 instruction, Mnemonic& mnemonic)
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
        offsetStr = "R" + std::to_string(flags.Rm);
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
        offsetStr = (offset == 0) ? "" : "#" + std::to_string(offset);
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

    std::tie(mnemonic.op, mnemonic.cond, mnemonic.args) =
        HalfwordDataTransferHelper(load, cond, Rd, Rn, s, h, up, preIndex, writeBack, offsetStr);
}

void DisassemblePSRTransferMRS(u32 instruction, Mnemonic& mnemonic)
{
    auto flags = std::bit_cast<PSRTransferMRS::Flags>(instruction);
    mnemonic.cond = DecodeCondition(flags.Cond);
    mnemonic.op = "MRS";

    std::string psr = flags.Ps ? "SPSR" : "CPSR";
    u8 Rd = flags.Rd;
    mnemonic.args = std::format("R{}, {}", Rd, psr);
}

void DisassemblePSRTransferMSR(u32 instruction, Mnemonic& mnemonic)
{
    auto flags = std::bit_cast<PSRTransferMSR::Flags>(instruction);
    mnemonic.cond = DecodeCondition(flags.Cond);
    mnemonic.op = "MSR";

    std::stringstream fields;
    fields << "_";
    fields << (flags.SetFlags ? "f" : "");
    fields << (flags.SetStatus ? "s" : "");
    fields << (flags.SetExtension ? "x" : "");
    fields << (flags.SetControl ? "c" : "");
    std::string fieldsStr = fields.str() == "_fsxc" ? "_all" : fields.str();

    std::string psr = flags.Pd ? "SPSR" : "CPSR";
    psr = psr + fieldsStr;

    if (flags.I)
    {
        auto immFlags = std::bit_cast<PSRTransferMSR::ImmSrc>(instruction);
        u32 imm = std::rotr(immFlags.Imm, immFlags.Rotate * 2);
        mnemonic.args = std::format("{}, #{:08X}", psr, imm);
    }
    else
    {
        auto regFlags = std::bit_cast<PSRTransferMSR::RegSrc>(instruction);
        u8 Rm = regFlags.Rm;
        mnemonic.args = std::format("{}, R{}", psr, Rm);
    }
}

void DisassembleDataProcessing(u32 instruction, Mnemonic& mnemonic)
{
    auto flags = std::bit_cast<DataProcessing::Flags>(instruction);
    mnemonic.cond = DecodeCondition(flags.Cond);

    switch (flags.OpCode)
    {
        case 0b0000:
            mnemonic.op = "AND";
            break;
        case 0b0001:
            mnemonic.op = "EOR";
            break;
        case 0b0010:
            mnemonic.op = "SUB";
            break;
        case 0b0011:
            mnemonic.op = "RSB";
            break;
        case 0b0100:
            mnemonic.op = "ADD";
            break;
        case 0b0101:
            mnemonic.op = "ADC";
            break;
        case 0b0110:
            mnemonic.op = "SBC";
            break;
        case 0b0111:
            mnemonic.op = "RSC";
            break;
        case 0b1000:
            mnemonic.op = "TST";
            break;
        case 0b1001:
            mnemonic.op = "TEQ";
            break;
        case 0b1010:
            mnemonic.op = "CMP";
            break;
        case 0b1011:
            mnemonic.op = "CMN";
            break;
        case 0b1100:
            mnemonic.op = "ORR";
            break;
        case 0b1101:
            mnemonic.op = "MOV";
            break;
        case 0b1110:
            mnemonic.op = "BIC";
            break;
        case 0b1111:
            mnemonic.op = "MVN";
            break;
    }

    if (flags.S && ((flags.OpCode < 8) || (flags.OpCode > 11)))
    {
        mnemonic.op += "S";
    }

    std::string op2Str;

    if (flags.I)
    {
        auto op2SrcFlags = std::bit_cast<DataProcessing::RotatedImmSrc>(instruction);
        u32 op2 = op2SrcFlags.Imm;
        u8 rotate = op2SrcFlags.Rotate << 1;
        op2 = std::rotr(op2, rotate);
        op2Str = std::format("#{:08X}", op2);
    }
    else
    {
        u8 shiftType;
        u8 shiftAmount = 0;
        u8 Rm;
        u8 Rs = 0;
        bool isRRX = false;

        if (flags.RegShift)
        {
            auto op2SrcFlags = std::bit_cast<DataProcessing::RegShiftedRegSrc>(instruction);
            shiftType = op2SrcFlags.ShiftOp;
            Rm = op2SrcFlags.Rm;
            Rs = op2SrcFlags.Rs;
        }
        else
        {
            auto op2SrcFlags = std::bit_cast<DataProcessing::ImmShiftedRegSrc>(instruction);
            shiftType = op2SrcFlags.ShiftOp;
            shiftAmount = op2SrcFlags.Imm;
            Rm = op2SrcFlags.Rm;
            isRRX = (op2SrcFlags.Imm == 0) && (op2SrcFlags.ShiftOp == 3);
        }

        std::string shiftTypeStr;

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
                shiftTypeStr = isRRX ? "RRX" : "ROR";
                break;
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

    u8 destIndex = flags.Rd;
    u8 op1Index = flags.Rn;

    switch (flags.OpCode)
    {
        case 0b1101:  // MOV
        case 0b1111:  // MVN
            mnemonic.args = std::format("R{}, {}", destIndex, op2Str);
            break;
        case 0b1010:  // CMP
        case 0b1011:  // CMN
        case 0b1001:  // TEQ
        case 0b1000:  // TST
            mnemonic.args = std::format("R{}, {}", op1Index, op2Str);
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
            mnemonic.args = std::format("R{}, R{}, {}", destIndex, op1Index, op2Str);
            break;
    }
}
}  // namespace cpu::arm
