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
    (void)instruction;
}
}  // namespace cpu
