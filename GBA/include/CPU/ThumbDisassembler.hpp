#pragma once

#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/Types/DebugTypes.hpp>
#include <GBA/include/Types/Types.hpp>

namespace cpu::thumb
{
using namespace debug::cpu;

/// @brief Disassemble a THUMB instruction into its human-readable mnemonic.
/// @param instruction Raw 16-bit THUMB instruction code.
/// @param addr Address of the instruction.
/// @return Disassembled instruction.
DisassembledInstruction DisassembleInstruction(u32 instruction, u32 addr);

///---------------------------------------------------------------------------------------------------------------------------------
/// @brief Functions to convert 16-bit THUMB instructions to their mnemonics and arguments.
///---------------------------------------------------------------------------------------------------------------------------------

void DisassembleSoftwareInterrupt(u16 instruction, DisassembledInstruction& disassembly);
void DisassembleUnconditionalBranch(u16 instruction, u32 pc, DisassembledInstruction& disassembly);
void DisassembleConditionalBranch(u16 instruction, u32 pc, DisassembledInstruction& disassembly);
void DisassembleMultipleLoadStore(u16 instruction, DisassembledInstruction& disassembly);
void DisassembleLongBranchWithLink(u16 instruction, DisassembledInstruction& disassembly);
void DisassembleAddOffsetToStackPointer(u16 instruction, DisassembledInstruction& disassembly);
void DisassemblePushPopRegisters(u16 instruction, DisassembledInstruction& disassembly);
void DisassembleLoadStoreHalfword(u16 instruction, DisassembledInstruction& disassembly);
void DisassembleSPRelativeLoadStore(u16 instruction, DisassembledInstruction& disassembly);
void DisassembleLoadAddress(u16 instruction, DisassembledInstruction& disassembly);
void DisassembleLoadStoreWithOffset(u16 instruction, DisassembledInstruction& disassembly);
void DisassembleLoadStoreSignExtendedByteHalfword(u16 instruction, DisassembledInstruction& disassembly);
void DisassemblePCRelativeLoad(u16 instruction, DisassembledInstruction& disassembly);
void DisassembleHiRegisterOperationsBranchExchange(u16 instruction, DisassembledInstruction& disassembly);
void DisassembleALUOperations(u16 instruction, DisassembledInstruction& disassembly);
void DisassembleMoveCompareAddSubtractImmediate(u16 instruction, DisassembledInstruction& disassembly);
void DisassembleAddSubtract(u16 instruction, DisassembledInstruction& disassembly);
void DisassembleMoveShiftedRegister(u16 instruction, DisassembledInstruction& disassembly);
}  // namespace cpu::thumb
