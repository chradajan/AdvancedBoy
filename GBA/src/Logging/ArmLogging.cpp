#include <GBA/include/CPU/ARM7TDMI.hpp>
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
std::string ConditionMnemonic(uint8_t condition)
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
using namespace arm;

void ARM7TDMI::LogBranchAndExchange(u32 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogBlockDataTransfer(u32 instruction) const
{
    (void)instruction;
}

void ARM7TDMI::LogBranch(u32 instruction) const
{
    Branch::Flags flags; std::memcpy(&flags, &instruction, sizeof(instruction));
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
    (void)instruction;
}
}  // namespace cpu
