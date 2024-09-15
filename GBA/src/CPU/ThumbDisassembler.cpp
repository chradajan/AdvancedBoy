#include <GBA/include/CPU/ThumbDisassembler.hpp>
#include <bit>
#include <format>
#include <sstream>
#include <string>
#include <GBA/include/CPU/ArmDisassembler.hpp>
#include <GBA/include/CPU/THUMB.hpp>
#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/Types/DebugTypes.hpp>
#include <GBA/include/Types/Types.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>

namespace
{
/// @brief Help write Push/Pop formatted register string.
/// @param regStream Stringstream to write to.
/// @param consecutiveRegisters How many consecutive registers were included in the operation.
/// @param regIndex Current index of register not included in operation.
void PushPopHelper(std::stringstream& regStream, int consecutiveRegisters, u8 regIndex)
{
    if (consecutiveRegisters <= 2)
    {
        for (int r = regIndex - consecutiveRegisters; r < regIndex; ++r)
        {
            regStream << "R" << r << ", ";
        }
    }
    else
    {
        regStream << "R" << (regIndex - consecutiveRegisters) << "-R" << (regIndex - 1) << ", ";
    }
}
}

namespace cpu::thumb
{
Mnemonic DisassembleInstruction(u16 instruction)
{
    Mnemonic mnemonic;
    mnemonic.cond = "";
    mnemonic.branchOffset = {};

    if (SoftwareInterrupt::IsInstanceOf(instruction))
    {
        DisassembleSoftwareInterrupt(instruction, mnemonic);
    }
    else if (UnconditionalBranch::IsInstanceOf(instruction))
    {
        DisassembleUnconditionalBranch(instruction, mnemonic);
    }
    else if (ConditionalBranch::IsInstanceOf(instruction))
    {
        DisassembleConditionalBranch(instruction, mnemonic);
    }
    else if (MultipleLoadStore::IsInstanceOf(instruction))
    {
        DisassembleMultipleLoadStore(instruction, mnemonic);
    }
    else if (LongBranchWithLink::IsInstanceOf(instruction))
    {
        DisassembleLongBranchWithLink(instruction, mnemonic);
    }
    else if (AddOffsetToStackPointer::IsInstanceOf(instruction))
    {
        DisassembleAddOffsetToStackPointer(instruction, mnemonic);
    }
    else if (PushPopRegisters::IsInstanceOf(instruction))
    {
        DisassemblePushPopRegisters(instruction, mnemonic);
    }
    else if (LoadStoreHalfword::IsInstanceOf(instruction))
    {
        DisassembleLoadStoreHalfword(instruction, mnemonic);
    }
    else if (SPRelativeLoadStore::IsInstanceOf(instruction))
    {
        DisassembleSPRelativeLoadStore(instruction, mnemonic);
    }
    else if (LoadAddress::IsInstanceOf(instruction))
    {
        DisassembleLoadAddress(instruction, mnemonic);
    }
    else if (LoadStoreWithImmOffset::IsInstanceOf(instruction))
    {
        DisassembleLoadStoreWithOffset(instruction, mnemonic);
    }
    else if (LoadStoreWithRegOffset::IsInstanceOf(instruction))
    {
        DisassembleLoadStoreWithOffset(instruction, mnemonic);
    }
    else if (LoadStoreSignExtendedByteHalfword::IsInstanceOf(instruction))
    {
        DisassembleLoadStoreSignExtendedByteHalfword(instruction, mnemonic);
    }
    else if (PCRelativeLoad::IsInstanceOf(instruction))
    {
        DisassemblePCRelativeLoad(instruction, mnemonic);
    }
    else if (HiRegisterOperationsBranchExchange::IsInstanceOf(instruction))
    {
        DisassembleHiRegisterOperationsBranchExchange(instruction, mnemonic);
    }
    else if (ALUOperations::IsInstanceOf(instruction))
    {
        DisassembleALUOperations(instruction, mnemonic);
    }
    else if (MoveCompareAddSubtractImmediate::IsInstanceOf(instruction))
    {
        DisassembleMoveCompareAddSubtractImmediate(instruction, mnemonic);
    }
    else if (AddSubtract::IsInstanceOf(instruction))
    {
        DisassembleAddSubtract(instruction, mnemonic);
    }
    else if (MoveShiftedRegister::IsInstanceOf(instruction))
    {
        DisassembleMoveShiftedRegister(instruction, mnemonic);
    }
    else
    {
        mnemonic.op = "???";
        mnemonic.cond = "";
        mnemonic.args = "???";
    }

    return mnemonic;
}

void DisassembleSoftwareInterrupt(u16 instruction, Mnemonic& mnemonic)
{
    auto flags = std::bit_cast<SoftwareInterrupt::Flags>(instruction);
    mnemonic.op = "SWI";

    u8 comment = flags.Value8;
    mnemonic.args = std::format("#{:02X}", comment);
}

void DisassembleUnconditionalBranch(u16 instruction, Mnemonic& mnemonic)
{
    auto flags = std::bit_cast<UnconditionalBranch::Flags>(instruction);
    mnemonic.op = "B";

    i16 offset = 4 + SignExtend<i16, 11>(flags.Offset11 << 1);
    mnemonic.branchOffset = offset;
    mnemonic.args = "#" + std::to_string(offset);
}

void DisassembleConditionalBranch(u16 instruction, Mnemonic& mnemonic)
{
    auto flags = std::bit_cast<ConditionalBranch::Flags>(instruction);
    mnemonic.cond = arm::DecodeCondition(flags.Cond);
    mnemonic.op = "B";

    i16 offset = 4 + SignExtend<i16, 8>(flags.SOffset8 << 1);
    mnemonic.branchOffset = offset;
    mnemonic.args = "#" + std::to_string(offset);
}

void DisassembleMultipleLoadStore(u16 instruction, Mnemonic& mnemonic)
{
    auto flags = std::bit_cast<MultipleLoadStore::Flags>(instruction);
    mnemonic.op = flags.L ? "LDMIA" : "STMIA";

    std::stringstream regStream;
    u8 regIndex = 0;
    u8 regList = flags.Rlist;
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
            PushPopHelper(regStream, consecutiveRegisters, regIndex);
            consecutiveRegisters = 0;
        }

        ++regIndex;
        regList >>= 1;
    }

    if (consecutiveRegisters > 0)
    {
        PushPopHelper(regStream, consecutiveRegisters, regIndex);
    }

    if (regStream.str().length() > 1)
    {
        regStream.seekp(-2, regStream.cur);
    }

    regStream << "}";
    u8 Rb = flags.Rb;
    mnemonic.args = std::format("R{}!, {}", Rb, regStream.str());
}

void DisassembleLongBranchWithLink(u16 instruction, Mnemonic& mnemonic)
{
    auto flags = std::bit_cast<LongBranchWithLink::Flags>(instruction);
    mnemonic.op = "BL";
    mnemonic.args = flags.H ? "1" : "0";
}

void DisassembleAddOffsetToStackPointer(u16 instruction, Mnemonic& mnemonic)
{
    auto flags = std::bit_cast<AddOffsetToStackPointer::Flags>(instruction);
    mnemonic.op = "ADD";

    u16 offset = flags.SWord7 << 2;
    mnemonic.args = std::format("SP, #{}{}", flags.S ? "-" : "", offset);
}

void DisassemblePushPopRegisters(u16 instruction, Mnemonic& mnemonic)
{
    auto flags = std::bit_cast<PushPopRegisters::Flags>(instruction);
    mnemonic.op = flags.L ? "POP" : "PUSH";

    std::string r = "";

    if (flags.R)
    {
        r = flags.L ? "PC" : "LR";
    }

    u8 regIndex = 0;
    std::stringstream regStream;
    u8 regList = flags.Rlist;
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
            PushPopHelper(regStream, consecutiveRegisters, regIndex);
            consecutiveRegisters = 0;
        }

        ++regIndex;
        regList >>= 1;
    }

    if (consecutiveRegisters > 0)
    {
        PushPopHelper(regStream, consecutiveRegisters, regIndex);
    }

    if (regStream.str().length() > 1)
    {
        if (r != "")
        {
            regStream << r;
        }
        else
        {
            regStream.seekp(-2, regStream.cur);
        }
    }
    else
    {
        regStream << r;
    }

    regStream << "}";
    mnemonic.args = regStream.str();
}

void DisassembleLoadStoreHalfword(u16 instruction, Mnemonic& mnemonic)
{
    auto flags = std::bit_cast<LoadStoreHalfword::Flags>(instruction);
    mnemonic.op = flags.L ? "LDRH" : "STRH";

    u8 Rd = flags.Rd;
    u8 Rb = flags.Rb;
    u8 offset = flags.Offset5 << 1;
    mnemonic.args = std::format("R{}, [R{}, #{}]", Rd, Rb, offset);
}

void DisassembleSPRelativeLoadStore(u16 instruction, Mnemonic& mnemonic)
{
    auto flags = std::bit_cast<SPRelativeLoadStore::Flags>(instruction);
    mnemonic.op = flags.L ? "LDR" : "STR";

    u8 Rd = flags.Rd;
    u16 imm = flags.Word8 << 2;
    mnemonic.args = std::format("R{}, [SP, #{}]", Rd, imm);
}

void DisassembleLoadAddress(u16 instruction, Mnemonic& mnemonic)
{
    auto flags = std::bit_cast<LoadAddress::Flags>(instruction);
    mnemonic.op = "ADD";

    std::string regStr = flags.SP ? "SP" : "PC";
    u16 offset = flags.Word8 << 2;
    u8 Rd = flags.Rd;
    mnemonic.args = std::format("R{}, {}, #{}", Rd, regStr, offset);
}

void DisassembleLoadStoreWithOffset(u16 instruction, Mnemonic& mnemonic)
{
    std::string op;
    std::string offsetStr;
    u8 Rd;
    u8 Rb;

    if (LoadStoreWithImmOffset::IsInstanceOf(instruction))
    {
        auto flags = std::bit_cast<LoadStoreWithImmOffset::Flags>(instruction);
        op = flags.L ? "LDR" : "STR";
        op += flags.B ? "B" : "";
        u8 offset = flags.B ? flags.Offset5 : (flags.Offset5 << 2);
        offsetStr = std::format("#{}", offset);
        Rd = flags.Rd;
        Rb = flags.Rb;
    }
    else
    {
        auto flags = std::bit_cast<LoadStoreWithRegOffset::Flags>(instruction);
        op = flags.L ? "LDR" : "STR";
        op += flags.B ? "B" : "";
        u8 Ro = flags.Ro;
        offsetStr = std::format("R{}", Ro);
        Rd = flags.Rd;
        Rb = flags.Rb;
    }

    mnemonic.op = op;
    mnemonic.args = std::format("R{}, [R{}, {}]", Rd, Rb, offsetStr);
}

void DisassembleLoadStoreSignExtendedByteHalfword(u16 instruction, Mnemonic& mnemonic)
{
    auto flags = std::bit_cast<LoadStoreSignExtendedByteHalfword::Flags>(instruction);

    u8 sh = (flags.S << 1) | (flags.H);

    switch (sh)
    {
        case 0b00:  // S = 0, H = 0
            mnemonic.op = "STRH";
            break;
        case 0b01:  // S = 0, H = 1
            mnemonic.op = "LDRH";
            break;
        case 0b10:  // S = 1, H = 0
            mnemonic.op = "LDSB";
            break;
        case 0b11:  // S = 1, H = 1
            mnemonic.op = "LDSH";
            break;
    }

    u8 Ro = flags.Ro;
    u8 Rb = flags.Rb;
    u8 Rd = flags.Rd;
    mnemonic.args = std::format("R{}, [R{}, R{}]", Rd, Rb, Ro);
}

void DisassemblePCRelativeLoad(u16 instruction, Mnemonic& mnemonic)
{
    auto flags = std::bit_cast<PCRelativeLoad::Flags>(instruction);
    mnemonic.op = "LDR";

    u8 Rd = flags.Rd;
    u16 offset = flags.Word8 << 2;
    mnemonic.args = std::format("R{}, [PC, #{}]", Rd, offset);
}

void DisassembleHiRegisterOperationsBranchExchange(u16 instruction, Mnemonic& mnemonic)
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
        case 0b00:
            mnemonic.op = "ADD";
            mnemonic.args = std::format("R{}, R{}", Rd, Rs);
            break;
        case 0b01:
            mnemonic.op = "CMP";
            mnemonic.args = std::format("R{}, R{}", Rd, Rs);
            break;
        case 0b10:
            mnemonic.op = "MOV";
            mnemonic.args = std::format("R{}, R{}", Rd, Rs);
            break;
        case 0b11:
            mnemonic.op = "BX";
            mnemonic.args = std::format("R{}", Rs);
            break;
    }
}

void DisassembleALUOperations(u16 instruction, Mnemonic& mnemonic)
{
    auto flags = std::bit_cast<ALUOperations::Flags>(instruction);

    switch (flags.Op)
    {
        case 0b0000:
            mnemonic.op = "AND";
            break;
        case 0b0001:
            mnemonic.op = "EOR";
            break;
        case 0b0010:
            mnemonic.op = "LSL";
            break;
        case 0b0011:
            mnemonic.op = "LSR";
            break;
        case 0b0100:
            mnemonic.op = "ASR";
            break;
        case 0b0101:
            mnemonic.op = "ADC";
            break;
        case 0b0110:
            mnemonic.op = "SBC";
            break;
        case 0b0111:
            mnemonic.op = "ROR";
            break;
        case 0b1000:
            mnemonic.op = "TST";
            break;
        case 0b1001:
            mnemonic.op = "NEG";
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
            mnemonic.op = "MUL";
            break;
        case 0b1110:
            mnemonic.op = "BIC";
            break;
        case 0b1111:
            mnemonic.op = "MVN";
            break;
    }

    u8 Rd = flags.Rd;
    u8 Rs = flags.Rs;
    mnemonic.args = std::format("R{}, R{}", Rd, Rs);
}

void DisassembleMoveCompareAddSubtractImmediate(u16 instruction, Mnemonic& mnemonic)
{
    auto flags = std::bit_cast<MoveCompareAddSubtractImmediate::Flags>(instruction);

    switch (flags.Op)
    {
        case 0b00:
            mnemonic.op = "MOV";
            break;
        case 0b01:
            mnemonic.op = "CMP";
            break;
        case 0b10:
            mnemonic.op = "ADD";
            break;
        case 0b11:
            mnemonic.op = "SUB";
            break;
    }

    u8 Rd = flags.Rd;
    u8 offset = flags.Offset8;
    mnemonic.args = std::format("R{}, #{}", Rd, offset);
}

void DisassembleAddSubtract(u16 instruction, Mnemonic& mnemonic)
{
    auto flags = std::bit_cast<AddSubtract::Flags>(instruction);

    u8 rnOffset = flags.RnOffset3;

    if (flags.I && (rnOffset == 0))
    {
        mnemonic.op = "MOV";
    }
    else
    {
        mnemonic.op = flags.Op ? "SUB" : "ADD";
    }

    std::string operand;

    if (!flags.I)
    {
        operand = std::format(", R{}", rnOffset);
    }
    else if (rnOffset > 0)
    {
        operand = std::format(", #{}", rnOffset);
    }

    u8 Rd = flags.Rd;
    u8 Rs = flags.Rs;
    mnemonic.args = std::format("R{}, R{}{}", Rd, Rs, operand);
}

void DisassembleMoveShiftedRegister(u16 instruction, Mnemonic& mnemonic)
{
    auto flags = std::bit_cast<MoveShiftedRegister::Flags>(instruction);

    switch (flags.Op)
    {
        case 0b00:
            mnemonic.op = "LSL";
            break;
        case 0b01:
            mnemonic.op = "LSR";
            break;
        case 0b10:
            mnemonic.op = "ASR";
            break;
        default:
            mnemonic.op = "???";
            break;
    }

    u8 Rd = flags.Rd;
    u8 Rs = flags.Rs;
    u8 offset = flags.Offset5;
    mnemonic.args = std::format("R{}, R{}, #{}", Rd, Rs, offset);
}
}  // namespace cpu::thumb
