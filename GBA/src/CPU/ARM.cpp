#include <GBA/include/CPU/ARM7TDMI.hpp>
#include <bit>
#include <cstring>
#include <stdexcept>
#include <GBA/include/CPU/ARM.hpp>
#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/Types.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>

namespace cpu
{
using namespace arm;

void ARM7TDMI::DecodeAndExecuteARM(u32 instruction, bool log)
{
    u8 conditionCode = (instruction & 0xF000'0000) >> 28;
    bool conditionMet = (conditionCode == 0x0E) || ConditionSatisfied(conditionCode);

    if (BranchAndExchange::IsInstanceOf(instruction))
    {
        if (log) LogBranchAndExchange(instruction);
        if (conditionMet) ExecuteBranchAndExchange(instruction);
    }
    else if (BlockDataTransfer::IsInstanceOf(instruction))
    {
        if (log) LogBlockDataTransfer(instruction);
        if (conditionMet) ExecuteBlockDataTransfer(instruction);
    }
    else if (Branch::IsInstanceOf(instruction))
    {
        if (log) LogBranch(instruction);
        if (conditionMet) ExecuteBranch(instruction);
    }
    else if (SoftwareInterrupt::IsInstanceOf(instruction))
    {
        if (log) LogArmSoftwareInterrupt(instruction);
        if (conditionMet) ExecuteArmSoftwareInterrupt(instruction);
    }
    else if (Undefined::IsInstanceOf(instruction))
    {
        if (log) LogUndefined(instruction);
        if (conditionMet) ExecuteUndefined(instruction);
    }
    else if (SingleDataTransfer::IsInstanceOf(instruction))
    {
        if (log) LogSingleDataTransfer(instruction);
        if (conditionMet) ExecuteSingleDataTransfer(instruction);
    }
    else if (SingleDataSwap::IsInstanceOf(instruction))
    {
        if (log) LogSingleDataSwap(instruction);
        if (conditionMet) ExecuteSingleDataSwap(instruction);
    }
    else if (Multiply::IsInstanceOf(instruction))
    {
        if (log) LogMultiply(instruction);
        if (conditionMet) ExecuteMultiply(instruction);
    }
    else if (MultiplyLong::IsInstanceOf(instruction))
    {
        if (log) LogMultiplyLong(instruction);
        if (conditionMet) ExecuteMultiplyLong(instruction);
    }
    else if (HalfwordDataTransferRegOffset::IsInstanceOf(instruction))
    {
        if (log) LogHalfwordDataTransferRegOffset(instruction);
        if (conditionMet) ExecuteHalfwordDataTransferRegOffset(instruction);
    }
    else if (HalfwordDataTransferImmOffset::IsInstanceOf(instruction))
    {
        if (log) LogHalfwordDataTransferImmOffset(instruction);
        if (conditionMet) ExecuteHalfwordDataTransferImmOffset(instruction);
    }
    else if (PSRTransferMRS::IsInstanceOf(instruction))
    {
        if (log) LogPSRTransferMRS(instruction);
        if (conditionMet) ExecutePSRTransferMRS(instruction);
    }
    else if (PSRTransferMSR::IsInstanceOf(instruction))
    {
        if (log) LogPSRTransferMSR(instruction);
        if (conditionMet) ExecutePSRTransferMSR(instruction);
    }
    else if (DataProcessing::IsInstanceOf(instruction))
    {
        if (log) LogDataProcessing(instruction);
        if (conditionMet) ExecuteDataProcessing(instruction);
    }
    else
    {
        throw std::runtime_error("Unable to decode ARM instruction");
    }
}

void ARM7TDMI::ExecuteBranchAndExchange(u32 instruction)
{
    (void)instruction;
    throw std::runtime_error("ExecuteBranchAndExchange not implemented");
}

void ARM7TDMI::ExecuteBlockDataTransfer(u32 instruction)
{
    auto flags = std::bit_cast<BlockDataTransfer::Flags>(instruction);

    u16 regList = flags.RegisterList;
    bool emptyRlist = regList == 0;
    bool wbIndexInList = flags.W && (regList & (0x01 << flags.Rn));
    bool wbIndexFirstInList = wbIndexInList && !flags.L && ((((0x01 << flags.Rn) - 1) & regList) == 0);

    OperatingMode mode = registers_.GetOperatingMode();
    bool r15InList = (regList & 0x8000) == 0x8000;

    if (regList == 0)
    {
        regList = 0x8000;
    }

    if (flags.S)
    {
        if (r15InList)
        {
            if (!flags.L)
            {
                mode = OperatingMode::User;
            }
        }
        else
        {
            mode = OperatingMode::User;
        }
    }

    int xferCnt = std::popcount(regList);
    u32 baseAddr = registers_.ReadRegister(flags.Rn);
    u32 minAddr = 0;
    u32 wbAddr = 0;
    bool preIndexOffset = flags.P;

    if (flags.U)
    {
        minAddr = preIndexOffset ? (baseAddr + 4) : baseAddr;

        if (preIndexOffset)
        {
            wbAddr = minAddr + (4 * (xferCnt - 1));
        }
        else
        {
            wbAddr = minAddr + (4 * xferCnt);
        }

        if (emptyRlist)
        {
            wbAddr = baseAddr + 0x40;
        }
    }
    else
    {
        if (preIndexOffset)
        {
            minAddr = baseAddr - (4 * xferCnt);
            wbAddr = minAddr;
        }
        else
        {
            minAddr = baseAddr - (4 * (xferCnt - 1));
            wbAddr = minAddr - 4;
        }

        if (emptyRlist)
        {
            minAddr = baseAddr - (preIndexOffset ? 0x40 : 0x3C);
            wbAddr = baseAddr - 0x40;
        }
    }

    u8 regIndex = 0;
    u32 addr = minAddr;

    while (regList != 0)
    {
        if (regList & 0x01)
        {
            if (flags.L)
            {
                auto [readValue, readCycles] = ReadMemory(addr, AccessSize::WORD);
                scheduler_.Step(readCycles);

                if (regIndex == PC_INDEX)
                {
                    flushPipeline_ = true;

                    if (flags.S)
                    {
                        registers_.LoadSPSR();
                    }
                }

                registers_.WriteRegister(regIndex, readValue, mode);
            }
            else
            {
                u32 regValue = registers_.ReadRegister(regIndex, mode);

                if (regIndex == PC_INDEX)
                {
                    regValue += 4;
                }
                else if ((regIndex == flags.Rn) && !wbIndexFirstInList)
                {
                    regValue = wbAddr;
                }

                int writeCycles = WriteMemory(addr, regValue, AccessSize::WORD);
                scheduler_.Step(writeCycles);
            }

            addr += 4;
        }

        ++regIndex;
        regList >>= 1;
    }

    if (flags.W && !(wbIndexInList && flags.L))
    {
        registers_.WriteRegister(flags.Rn, wbAddr);
    }

    if (flags.L)
    {
        scheduler_.Step(1);
    }
}

void ARM7TDMI::ExecuteBranch(u32 instruction)
{
    auto flags = std::bit_cast<Branch::Flags>(instruction);
    i32 offset = SignExtend<i32, 25>(flags.Offset << 2);

    if (flags.L)
    {
        registers_.WriteRegister(LR_INDEX, (registers_.GetPC() - 4) & 0xFFFF'FFFC);
    }

    registers_.SetPC(registers_.GetPC() + offset);
    flushPipeline_ = true;
}

void ARM7TDMI::ExecuteArmSoftwareInterrupt(u32 instruction)
{
    (void)instruction;
    throw std::runtime_error("ArmSoftwareInterrupt not implemented");
}

void ARM7TDMI::ExecuteUndefined(u32 instruction)
{
    (void)instruction;
    throw std::runtime_error("Undefined not implemented");
}

void ARM7TDMI::ExecuteSingleDataTransfer(u32 instruction)
{
    (void)instruction;
    throw std::runtime_error("SingleDataTransfer not implemented");
}

void ARM7TDMI::ExecuteSingleDataSwap(u32 instruction)
{
    (void)instruction;
    throw std::runtime_error("SingleDataSwap not implemented");
}

void ARM7TDMI::ExecuteMultiply(u32 instruction)
{
    (void)instruction;
    throw std::runtime_error("Multiply not implemented");
}

void ARM7TDMI::ExecuteMultiplyLong(u32 instruction)
{
    (void)instruction;
    throw std::runtime_error("MultiplyLong not implemented");
}

void ARM7TDMI::ExecuteHalfwordDataTransferRegOffset(u32 instruction)
{
    (void)instruction;
    throw std::runtime_error("HalfwordDataTransferRegOffset not implemented");
}

void ARM7TDMI::ExecuteHalfwordDataTransferImmOffset(u32 instruction)
{
    (void)instruction;
    throw std::runtime_error("HalfwordDataTransferImmOffset not implemented");
}

void ARM7TDMI::ExecutePSRTransferMRS(u32 instruction)
{
    (void)instruction;
    throw std::runtime_error("PSRTransferMRS not implemented");
}

void ARM7TDMI::ExecutePSRTransferMSR(u32 instruction)
{
    (void)instruction;
    throw std::runtime_error("PSRTransferMSR not implemented");
}

void ARM7TDMI::ExecuteDataProcessing(u32 instruction)
{
    (void)instruction;
    throw std::runtime_error("DataProcessing not implemented");
}
}  // namespace cpu::arm
