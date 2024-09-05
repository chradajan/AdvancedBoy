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
DisassembledInstruction DisassembleInstruction(u32 instruction, u32 addr)
{
    DisassembledInstruction disassembly;
    disassembly.armInstruction = false;
    disassembly.undecodedInstruction = instruction & U16_MAX;
    disassembly.addr = addr;

    if (SoftwareInterrupt::IsInstanceOf(instruction))
    {
        DisassembleSoftwareInterrupt(instruction, disassembly);
    }
    else if (UnconditionalBranch::IsInstanceOf(instruction))
    {
        DisassembleUnconditionalBranch(instruction, addr, disassembly);
    }
    else if (ConditionalBranch::IsInstanceOf(instruction))
    {
        DisassembleConditionalBranch(instruction, addr, disassembly);
    }
    else if (MultipleLoadStore::IsInstanceOf(instruction))
    {
        DisassembleMultipleLoadStore(instruction, disassembly);
    }
    else if (LongBranchWithLink::IsInstanceOf(instruction))
    {
        DisassembleLongBranchWithLink(instruction, disassembly);
    }
    else if (AddOffsetToStackPointer::IsInstanceOf(instruction))
    {
        DisassembleAddOffsetToStackPointer(instruction, disassembly);
    }
    else if (PushPopRegisters::IsInstanceOf(instruction))
    {
        DisassemblePushPopRegisters(instruction, disassembly);
    }
    else if (LoadStoreHalfword::IsInstanceOf(instruction))
    {
        DisassembleLoadStoreHalfword(instruction, disassembly);
    }
    else if (SPRelativeLoadStore::IsInstanceOf(instruction))
    {
        DisassembleSPRelativeLoadStore(instruction, disassembly);
    }
    else if (LoadAddress::IsInstanceOf(instruction))
    {
        DisassembleLoadAddress(instruction, disassembly);
    }
    else if (LoadStoreWithImmOffset::IsInstanceOf(instruction))
    {
        DisassembleLoadStoreWithOffset(instruction, disassembly);
    }
    else if (LoadStoreWithRegOffset::IsInstanceOf(instruction))
    {
        DisassembleLoadStoreWithOffset(instruction, disassembly);
    }
    else if (LoadStoreSignExtendedByteHalfword::IsInstanceOf(instruction))
    {
        DisassembleLoadStoreSignExtendedByteHalfword(instruction, disassembly);
    }
    else if (PCRelativeLoad::IsInstanceOf(instruction))
    {
        DisassemblePCRelativeLoad(instruction, disassembly);
    }
    else if (HiRegisterOperationsBranchExchange::IsInstanceOf(instruction))
    {
        DisassembleHiRegisterOperationsBranchExchange(instruction, disassembly);
    }
    else if (ALUOperations::IsInstanceOf(instruction))
    {
        DisassembleALUOperations(instruction, disassembly);
    }
    else if (MoveCompareAddSubtractImmediate::IsInstanceOf(instruction))
    {
        DisassembleMoveCompareAddSubtractImmediate(instruction, disassembly);
    }
    else if (AddSubtract::IsInstanceOf(instruction))
    {
        DisassembleAddSubtract(instruction, disassembly);
    }
    else if (MoveShiftedRegister::IsInstanceOf(instruction))
    {
        DisassembleMoveShiftedRegister(instruction, disassembly);
    }
    else
    {
        disassembly.op = "???";
        disassembly.cond = "";
        disassembly.args = "???";
    }

    return disassembly;
}

void DisassembleSoftwareInterrupt(u16 instruction, DisassembledInstruction& disassembly)
{
    auto flags = std::bit_cast<SoftwareInterrupt::Flags>(instruction);
    disassembly.op = "SWI";

    u8 comment = flags.Value8;
    disassembly.args = std::format("#{:02X}", comment);
}

void DisassembleUnconditionalBranch(u16 instruction, u32 pc, DisassembledInstruction& disassembly)
{
    auto flags = std::bit_cast<UnconditionalBranch::Flags>(instruction);
    disassembly.op = "B";

    pc = pc + 4 + SignExtend<i16, 11>(flags.Offset11 << 1);
    disassembly.args = std::format("#{:08X}", pc);
}

void DisassembleConditionalBranch(u16 instruction, u32 pc, DisassembledInstruction& disassembly)
{
    auto flags = std::bit_cast<ConditionalBranch::Flags>(instruction);
    disassembly.cond = arm::DecodeCondition(flags.Cond);
    disassembly.op = "B";

    pc = pc + 4 + SignExtend<i16, 8>(flags.SOffset8 << 1);
    disassembly.args = std::format("#{:08X}", pc);
}

void DisassembleMultipleLoadStore(u16 instruction, DisassembledInstruction& disassembly)
{
    auto flags = std::bit_cast<MultipleLoadStore::Flags>(instruction);
    disassembly.op = flags.L ? "LDMIA" : "STMIA";

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
    disassembly.args = std::format("R{}!, {}", Rb, regStream.str());
}

void DisassembleLongBranchWithLink(u16 instruction, DisassembledInstruction& disassembly)
{
    auto flags = std::bit_cast<LongBranchWithLink::Flags>(instruction);
    disassembly.op = "BL";
    disassembly.args = flags.H ? "1" : "0";
}

void DisassembleAddOffsetToStackPointer(u16 instruction, DisassembledInstruction& disassembly)
{
    auto flags = std::bit_cast<AddOffsetToStackPointer::Flags>(instruction);
    disassembly.op = "ADD";

    u16 offset = flags.SWord7 << 2;
    disassembly.args = std::format("SP, #{}{}", flags.S ? "-" : "", offset);
}

void DisassemblePushPopRegisters(u16 instruction, DisassembledInstruction& disassembly)
{
    auto flags = std::bit_cast<PushPopRegisters::Flags>(instruction);
    disassembly.op = flags.L ? "POP" : "PUSH";

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
    disassembly.args = regStream.str();
}

void DisassembleLoadStoreHalfword(u16 instruction, DisassembledInstruction& disassembly)
{
    auto flags = std::bit_cast<LoadStoreHalfword::Flags>(instruction);
    disassembly.op = flags.L ? "LDRH" : "STRH";

    u8 Rd = flags.Rd;
    u8 Rb = flags.Rb;
    u8 offset = flags.Offset5 << 1;
    disassembly.args = std::format("R{}, [R{}, #{}]", Rd, Rb, offset);
}

void DisassembleSPRelativeLoadStore(u16 instruction, DisassembledInstruction& disassembly)
{
    disassembly.armInstruction = false;
    disassembly.undecodedInstruction = instruction & U16_MAX;
    disassembly.cond = "";
    auto flags = std::bit_cast<SPRelativeLoadStore::Flags>(instruction);
    disassembly.op = flags.L ? "LDR" : "STR";

    u8 Rd = flags.Rd;
    u16 imm = flags.Word8 << 2;
    disassembly.args = std::format("R{}, [SP, #{}]", Rd, imm);
}

void DisassembleLoadAddress(u16 instruction, DisassembledInstruction& disassembly)
{
    auto flags = std::bit_cast<LoadAddress::Flags>(instruction);
    disassembly.op = "ADD";

    std::string regStr = flags.SP ? "SP" : "PC";
    u16 offset = flags.Word8 << 2;
    u8 Rd = flags.Rd;
    disassembly.args = std::format("R{}, {}, #{}", Rd, regStr, offset);
}

void DisassembleLoadStoreWithOffset(u16 instruction, DisassembledInstruction& disassembly)
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

    disassembly.op = op;
    disassembly.args = std::format("R{}, [R{}, {}]", Rd, Rb, offsetStr);
}

void DisassembleLoadStoreSignExtendedByteHalfword(u16 instruction, DisassembledInstruction& disassembly)
{
    auto flags = std::bit_cast<LoadStoreSignExtendedByteHalfword::Flags>(instruction);

    u8 sh = (flags.S << 1) | (flags.H);

    switch (sh)
    {
        case 0b00:  // S = 0, H = 0
            disassembly.op = "STRH";
            break;
        case 0b01:  // S = 0, H = 1
            disassembly.op = "LDRH";
            break;
        case 0b10:  // S = 1, H = 0
            disassembly.op = "LDSB";
            break;
        case 0b11:  // S = 1, H = 1
            disassembly.op = "LDSH";
            break;
    }

    u8 Ro = flags.Ro;
    u8 Rb = flags.Rb;
    u8 Rd = flags.Rd;
    disassembly.args = std::format("R{}, [R{}, R{}]", Rd, Rb, Ro);
}

void DisassemblePCRelativeLoad(u16 instruction, DisassembledInstruction& disassembly)
{
    auto flags = std::bit_cast<PCRelativeLoad::Flags>(instruction);
    disassembly.op = "LDR";

    u8 Rd = flags.Rd;
    u16 offset = flags.Word8 << 2;
    disassembly.args = std::format("R{}, [PC, #{}]", Rd, offset);
}

void DisassembleHiRegisterOperationsBranchExchange(u16 instruction, DisassembledInstruction& disassembly)
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
            disassembly.op = "ADD";
            disassembly.args = std::format("R{}, R{}", Rd, Rs);
            break;
        case 0b01:
            disassembly.op = "CMP";
            disassembly.args = std::format("R{}, R{}", Rd, Rs);
            break;
        case 0b10:
            disassembly.op = "MOV";
            disassembly.args = std::format("R{}, R{}", Rd, Rs);
            break;
        case 0b11:
            disassembly.op = "BX";
            disassembly.args = std::format("R{}", Rs);
            break;
    }
}

void DisassembleALUOperations(u16 instruction, DisassembledInstruction& disassembly)
{
    auto flags = std::bit_cast<ALUOperations::Flags>(instruction);

    switch (flags.Op)
    {
        case 0b0000:
            disassembly.op = "AND";
            break;
        case 0b0001:
            disassembly.op = "EOR";
            break;
        case 0b0010:
            disassembly.op = "LSL";
            break;
        case 0b0011:
            disassembly.op = "LSR";
            break;
        case 0b0100:
            disassembly.op = "ASR";
            break;
        case 0b0101:
            disassembly.op = "ADC";
            break;
        case 0b0110:
            disassembly.op = "SBC";
            break;
        case 0b0111:
            disassembly.op = "ROR";
            break;
        case 0b1000:
            disassembly.op = "TST";
            break;
        case 0b1001:
            disassembly.op = "NEG";
            break;
        case 0b1010:
            disassembly.op = "CMP";
            break;
        case 0b1011:
            disassembly.op = "CMN";
            break;
        case 0b1100:
            disassembly.op = "ORR";
            break;
        case 0b1101:
            disassembly.op = "MUL";
            break;
        case 0b1110:
            disassembly.op = "BIC";
            break;
        case 0b1111:
            disassembly.op = "MVN";
            break;
    }

    u8 Rd = flags.Rd;
    u8 Rs = flags.Rs;
    disassembly.args = std::format("R{}, R{}", Rd, Rs);
}

void DisassembleMoveCompareAddSubtractImmediate(u16 instruction, DisassembledInstruction& disassembly)
{
    auto flags = std::bit_cast<MoveCompareAddSubtractImmediate::Flags>(instruction);

    switch (flags.Op)
    {
        case 0b00:
            disassembly.op = "MOV";
            break;
        case 0b01:
            disassembly.op = "CMP";
            break;
        case 0b10:
            disassembly.op = "ADD";
            break;
        case 0b11:
            disassembly.op = "SUB";
            break;
    }

    u8 Rd = flags.Rd;
    u8 offset = flags.Offset8;
    disassembly.args = std::format("R{}, #{}", Rd, offset);
}

void DisassembleAddSubtract(u16 instruction, DisassembledInstruction& disassembly)
{
    auto flags = std::bit_cast<AddSubtract::Flags>(instruction);

    u8 rnOffset = flags.RnOffset3;

    if (flags.I && (rnOffset == 0))
    {
        disassembly.op = "MOV";
    }
    else
    {
        disassembly.op = flags.Op ? "SUB" : "ADD";
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
    disassembly.args = std::format("R{}, R{}{}", Rd, Rs, operand);
}

void DisassembleMoveShiftedRegister(u16 instruction, DisassembledInstruction& disassembly)
{
    auto flags = std::bit_cast<MoveShiftedRegister::Flags>(instruction);

    switch (flags.Op)
    {
        case 0b00:
            disassembly.op = "LSL";
            break;
        case 0b01:
            disassembly.op = "LSR";
            break;
        case 0b10:
            disassembly.op = "ASR";
            break;
        default:
            disassembly.op = "???";
            break;
    }

    u8 Rd = flags.Rd;
    u8 Rs = flags.Rs;
    u8 offset = flags.Offset5;
    disassembly.args = std::format("R{}, R{}, #{}", Rd, Rs, offset);
}
}  // namespace cpu::thumb
