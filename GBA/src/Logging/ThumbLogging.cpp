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
    (void)instruction;
}

void ARM7TDMI::LogConditionalBranch(u16 instruction) const
{
    auto flags = std::bit_cast<ConditionalBranch::Flags>(instruction);
    std::string cond = ConditionMnemonic(flags.Cond);
    u16 unsignedOffset = flags.SOffset8 << 1;
    i16 signedOffset = SignExtend<i16, 8>(unsignedOffset);
    u32 pc = registers_.GetPC() + signedOffset;
    std::string mnemonic = std::format("{:04X} -> B{} 0x{:08X}", instruction, cond, pc);
    log_.LogCPU(mnemonic, registers_.RegistersString(), logPC_);
}

void ARM7TDMI::LogMultipleLoadStore(u16 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogLongBranchWithLink(u16 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogAddOffsetToStackPointer(u16 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogPushPopRegisters(u16 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogLoadStoreHalfword(u16 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogSPRelativeLoadStore(u16 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogLoadAddress(u16 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogLoadStoreWithImmOffset(u16 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogLoadStoreWithRegOffset(u16 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogLoadStoreSignExtendedByteHalfword(u16 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogPCRelativeLoad(u16 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogHiRegisterOperationsBranchExchange(u16 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogALUOperations(u16 instruction) const
{
    (void)instruction;
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
    std::string mnemonic = std::format("{:04X} -> {} R{}, #{}", instruction, op, Rd, offset);
    log_.LogCPU(mnemonic, registers_.RegistersString(), logPC_);
}

void ARM7TDMI::LogAddSubtract(u16 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogMoveShiftedRegister(u16 instruction) const
{
    (void)instruction;
}
}  // namespace cpu
