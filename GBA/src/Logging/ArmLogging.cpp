#include <GBA/include/CPU/ARM7TDMI.hpp>
#include <GBA/include/CPU/ARM.hpp>
#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/Types.hpp>

namespace cpu
{
void ARM7TDMI::LogBranchAndExchange(u32 val)
{
    (void)val;
}

void ARM7TDMI::LogBlockDataTransfer(u32 val)
{
    (void)val;
}

void ARM7TDMI::LogBranch(u32 instruction)
{
    (void)instruction;
}

void ARM7TDMI::LogArmSoftwareInterrupt(u32 instruction)
{
    (void)instruction;
}

void ARM7TDMI::LogUndefined(u32 instruction)
{
    (void)instruction;
}

void ARM7TDMI::LogSingleDataTransfer(u32 instruction)
{
    (void)instruction;
}

void ARM7TDMI::LogSingleDataSwap(u32 instruction)
{
    (void)instruction;
}

void ARM7TDMI::LogMultiply(u32 instruction)
{
    (void)instruction;
}

void ARM7TDMI::LogMultiplyLong(u32 instruction)
{
    (void)instruction;
}

void ARM7TDMI::LogHalfwordDataTransferRegOffset(u32 instruction)
{
    (void)instruction;
}

void ARM7TDMI::LogHalfwordDataTransferImmOffset(u32 instruction)
{
    (void)instruction;
}

void ARM7TDMI::LogPSRTransferMRS(u32 instruction)
{
    (void)instruction;
}

void ARM7TDMI::LogPSRTransferMSR(u32 instruction)
{
    (void)instruction;
}

void ARM7TDMI::LogDataProcessing(u32 instruction)
{
    (void)instruction;
}
}  // namespace cpu
