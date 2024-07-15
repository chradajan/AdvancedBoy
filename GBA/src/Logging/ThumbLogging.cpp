#include <GBA/include/CPU/ARM7TDMI.hpp>
#include <GBA/include/CPU/THUMB.hpp>
#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/Types.hpp>

namespace cpu
{
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
    (void)instruction;
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
    (void)instruction;
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
