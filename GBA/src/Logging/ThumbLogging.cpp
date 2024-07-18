#include <GBA/include/CPU/ARM7TDMI.hpp>
#include <bit>
#include <format>
#include <sstream>
#include <string>
#include <GBA/include/CPU/THUMB.hpp>
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

namespace cpu
{
using namespace thumb;

void ARM7TDMI::LogThumbSoftwareInterrupt(u16 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogUnconditionalBranch(u16 instruction) const
{
    auto flags = std::bit_cast<UnconditionalBranch::Flags>(instruction);
    u32 pc = registers_.GetPC() + SignExtend<i16, 11>(flags.Offset11 << 1);
    std::string mnemonic = std::format("    {:04X} -> B #{:08X}", instruction, pc);
    log_.LogCPU(mnemonic, registers_.RegistersString(), logPC_);
}

void ARM7TDMI::LogConditionalBranch(u16 instruction) const
{
    auto flags = std::bit_cast<ConditionalBranch::Flags>(instruction);
    std::string cond = ConditionMnemonic(flags.Cond);
    u16 offset = flags.SOffset8 << 1;
    offset = SignExtend<i16, 8>(offset);
    u32 pc = registers_.GetPC() + offset;
    std::string mnemonic = std::format("    {:04X} -> B{} 0x{:08X}", instruction, cond, pc);
    log_.LogCPU(mnemonic, registers_.RegistersString(), logPC_);
}

void ARM7TDMI::LogMultipleLoadStore(u16 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogLongBranchWithLink(u16 instruction) const
{
    auto flags = std::bit_cast<LongBranchWithLink::Flags>(instruction);
    std::string mnemonic = std::format("    {:04X} -> BL {}", instruction, flags.H ? "1" : "0");
    log_.LogCPU(mnemonic, registers_.RegistersString(), logPC_);
}

void ARM7TDMI::LogAddOffsetToStackPointer(u16 instruction) const
{
    auto flags = std::bit_cast<AddOffsetToStackPointer::Flags>(instruction);
    u16 offset = flags.SWord7 << 2;
    std::string imm = std::format("#{}{}", flags.S ? "-" : "", offset);
    std::string mnemonic = std::format("    {:04X} -> ADD SP, {}", instruction, imm);
    log_.LogCPU(mnemonic, registers_.RegistersString(), logPC_);
}

void ARM7TDMI::LogPushPopRegisters(u16 instruction) const
{
    auto flags = std::bit_cast<PushPopRegisters::Flags>(instruction);
    std::string op = flags.L ? "POP" : "PUSH";
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
    std::string mnemonic = std::format("    {:04X} -> {} {}", instruction, op, regStream.str());
}

void ARM7TDMI::LogLoadStoreHalfword(u16 instruction) const
{
    auto flags = std::bit_cast<LoadStoreHalfword::Flags>(instruction);
    u8 Rd = flags.Rd;
    u8 Rb = flags.Rb;
    u8 offset = flags.Offset5 << 1;
    std::string op = flags.L ? "LDRH" : "STRH";
    std::string mnemonic = std::format("    {:04X} -> {} R{}, [R{}, #{}]", instruction, op, Rd, Rb, offset);
    log_.LogCPU(mnemonic, registers_.RegistersString(), logPC_);
}

void ARM7TDMI::LogSPRelativeLoadStore(u16 instruction) const
{
    auto flags = std::bit_cast<SPRelativeLoadStore::Flags>(instruction);
    std::string op = flags.L ? "LDR" : "STR";
    u8 Rd = flags.Rd;
    u16 imm = flags.Word8 << 2;
    std::string mnemonic = std::format("    {:04X} -> {} R{}, [SP, #{}]", instruction, op, Rd, imm);
    log_.LogCPU(mnemonic, registers_.RegistersString(), logPC_);
}

void ARM7TDMI::LogLoadAddress(u16 instruction) const
{
    auto flags = std::bit_cast<LoadAddress::Flags>(instruction);
    std::string regStr = flags.SP ? "SP" : "PC";
    u16 offset = flags.Word8 << 2;
    u8 Rd = flags.Rd;
    std::string mnemonic = std::format("    {:04X} -> ADD R{}, {}, #{}", instruction, Rd, regStr, offset);
    log_.LogCPU(mnemonic, registers_.RegistersString(), logPC_);
}

void ARM7TDMI::LogLoadStoreWithOffset(u16 instruction) const
{
    std::string op;
    std::string b;
    std::string offsetStr;
    u8 Rd;
    u8 Rb;

    if (LoadStoreWithImmOffset::IsInstanceOf(instruction))
    {
        auto flags = std::bit_cast<LoadStoreWithImmOffset::Flags>(instruction);
        op = flags.L ? "LDR" : "STR";
        b = flags.B ? "B" : "";
        u8 offset = flags.B ? flags.Offset5 : (flags.Offset5 << 2);
        offsetStr = std::format("#{}", offset);
        Rd = flags.Rd;
        Rb = flags.Rb;
    }
    else
    {
        auto flags = std::bit_cast<LoadStoreWithRegOffset::Flags>(instruction);
        op = flags.L ? "LDR" : "STR";
        b = flags.B ? "B" : "";
        u8 Ro = flags.Ro;
        offsetStr = std::format("R{}", Ro);
        Rd = flags.Rd;
        Rb = flags.Rb;
    }

    std::string mnemonic = std::format("    {:04X} -> {} R{}, [R{}, {}]", instruction, op, Rd, Rb, offsetStr);
    log_.LogCPU(mnemonic, registers_.RegistersString(), logPC_);
}

void ARM7TDMI::LogLoadStoreSignExtendedByteHalfword(u16 instruction) const
{
    auto flags = std::bit_cast<LoadStoreSignExtendedByteHalfword::Flags>(instruction);
    u8 Ro = flags.Ro;
    u8 Rb = flags.Rb;
    u8 Rd = flags.Rd;
    std::string regString = std::format("R{}, [R{}, R{}]", Rd, Rb, Ro);

    std::string op;
    u8 sh = (flags.S << 1) | (flags.H);

    switch (sh)
    {
        case 0b00:  // S = 0, H = 0
            op = "STRH";
            break;
        case 0b01:  // S = 0, H = 1
            op = "LDRH";
            break;
        case 0b10:  // S = 1, H = 0
            op = "LDSB";
            break;
        case 0b11:  // S = 1, H = 1
            op = "LDSH";
            break;
    }

    std::string mnemonic = std::format("    {:04X} -> {} {}", instruction, op, regString);
    log_.LogCPU(mnemonic, registers_.RegistersString(), logPC_);
}

void ARM7TDMI::LogPCRelativeLoad(u16 instruction) const
{
    auto flags = std::bit_cast<PCRelativeLoad::Flags>(instruction);
    u8 Rd = flags.Rd;
    u16 offset = flags.Word8 << 2;
    std::string mnemonic = std::format("    {:04X} -> LDR R{}, [PC, #{}]",instruction, Rd, offset);
    log_.LogCPU(mnemonic, registers_.RegistersString(), logPC_);
}

void ARM7TDMI::LogHiRegisterOperationsBranchExchange(u16 instruction) const
{
    auto flags = std::bit_cast<HiRegisterOperationsBranchExchange::Flags>(instruction);
    u8 Rd = flags.RdHd;
    u8 Rs = flags.RsHs;
    std::string op;
    std::string regStr = std::format("R{}, R{}", Rd, Rs);

    switch (flags.Op)
    {
        case 0b00:
            op = "ADD";
            break;
        case 0b01:
            op = "CMP";
            break;
        case 0b10:
            op = "MOV";
            break;
        case 0b11:
            op = "BX";
            regStr = std::format("R{}", Rs);
            break;
    }

    std::string mnemonic = std::format("    {:04X} -> {} {}", instruction, op, regStr);
    log_.LogCPU(mnemonic, registers_.RegistersString(), logPC_);
}

void ARM7TDMI::LogALUOperations(u16 instruction) const
{
    auto flags = std::bit_cast<ALUOperations::Flags>(instruction);
    std::string op;

    switch (flags.Op)
    {
        case 0b0000:
            op = "AND";
            break;
        case 0b0001:
            op = "EOR";
            break;
        case 0b0010:
            op = "LSL";
            break;
        case 0b0011:
            op = "LSR";
            break;
        case 0b0100:
            op = "ASR";
            break;
        case 0b0101:
            op = "ADC";
            break;
        case 0b0110:
            op = "SBC";
            break;
        case 0b0111:
            op = "ROR";
            break;
        case 0b1000:
            op = "TST";
            break;
        case 0b1001:
            op = "NEG";
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
            op = "MUL";
            break;
        case 0b1110:
            op = "BIC";
            break;
        case 0b1111:
            op = "MVN";
            break;
    }

    u8 Rd = flags.Rd;
    u8 Rs = flags.Rs;
    std::string mnemonic = std::format("    {:04X} -> {} R{}, R{}", instruction, op, Rd, Rs);
    log_.LogCPU(mnemonic, registers_.RegistersString(), logPC_);
}

void ARM7TDMI::LogMoveCompareAddSubtractImmediate(u16 instruction) const
{
    auto flags = std::bit_cast<MoveCompareAddSubtractImmediate::Flags>(instruction);
    std::string op;

    switch (flags.Op)
    {
        case 0b00:
            op = "MOV";
            break;
        case 0b01:
            op = "CMP";
            break;
        case 0b10:
            op = "ADD";
            break;
        case 0b11:
            op = "SUB";
            break;
    }

    u8 Rd = flags.Rd;
    u8 offset = flags.Offset8;
    std::string mnemonic = std::format("    {:04X} -> {} R{}, #{}", instruction, op, Rd, offset);
    log_.LogCPU(mnemonic, registers_.RegistersString(), logPC_);
}

void ARM7TDMI::LogAddSubtract(u16 instruction) const
{
    auto flags = std::bit_cast<AddSubtract::Flags>(instruction);
    std::string op;
    u8 rnOffset = flags.RnOffset3;

    if (flags.I && (rnOffset == 0))
    {
        op = "MOV";
    }
    else
    {
        op = flags.Op ? "SUB" : "ADD";
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
    std::string mnemonic = std::format("    {:04X} -> {} R{}, R{}{}", instruction, op, Rd, Rs, operand);
    log_.LogCPU(mnemonic, registers_.RegistersString(), logPC_);
}

void ARM7TDMI::LogMoveShiftedRegister(u16 instruction) const
{
    auto flags = std::bit_cast<MoveShiftedRegister::Flags>(instruction);
    std::string op;

    switch (flags.Op)
    {
        case 0b00:
            op = "LSL";
            break;
        case 0b01:
            op = "LSR";
            break;
        case 0b10:
            op = "ASR";
            break;
    }

    u8 Rd = flags.Rd;
    u8 Rs = flags.Rs;
    u8 offset = flags.Offset5;

    std::string mnemonic = std::format("    {:04X} -> {} R{}, R{}, #{}", instruction, op, Rd, Rs, offset);
    log_.LogCPU(mnemonic, registers_.RegistersString(), logPC_);
}
}  // namespace cpu
