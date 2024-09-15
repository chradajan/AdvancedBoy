#pragma once

#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/Types/DebugTypes.hpp>
#include <GBA/include/Types/Types.hpp>

namespace cpu::thumb
{
using namespace debug::cpu;

/// @brief Disassemble a THUMB instruction into its human-readable mnemonic.
/// @param instruction Raw 16-bit THUMB instruction code.
/// @return Disassembled instruction.
Mnemonic DisassembleInstruction(u16 instruction);

///---------------------------------------------------------------------------------------------------------------------------------
/// @brief Functions to convert 16-bit THUMB instructions to their mnemonics and arguments.
///---------------------------------------------------------------------------------------------------------------------------------

void DisassembleSoftwareInterrupt(u16 instruction, Mnemonic& mnemonic);
void DisassembleUnconditionalBranch(u16 instruction, Mnemonic& mnemonic);
void DisassembleConditionalBranch(u16 instruction, Mnemonic& mnemonic);
void DisassembleMultipleLoadStore(u16 instruction, Mnemonic& mnemonic);
void DisassembleLongBranchWithLink(u16 instruction, Mnemonic& mnemonic);
void DisassembleAddOffsetToStackPointer(u16 instruction, Mnemonic& mnemonic);
void DisassemblePushPopRegisters(u16 instruction, Mnemonic& mnemonic);
void DisassembleLoadStoreHalfword(u16 instruction, Mnemonic& mnemonic);
void DisassembleSPRelativeLoadStore(u16 instruction, Mnemonic& mnemonic);
void DisassembleLoadAddress(u16 instruction, Mnemonic& mnemonic);
void DisassembleLoadStoreWithOffset(u16 instruction, Mnemonic& mnemonic);
void DisassembleLoadStoreSignExtendedByteHalfword(u16 instruction, Mnemonic& mnemonic);
void DisassemblePCRelativeLoad(u16 instruction, Mnemonic& mnemonic);
void DisassembleHiRegisterOperationsBranchExchange(u16 instruction, Mnemonic& mnemonic);
void DisassembleALUOperations(u16 instruction, Mnemonic& mnemonic);
void DisassembleMoveCompareAddSubtractImmediate(u16 instruction, Mnemonic& mnemonic);
void DisassembleAddSubtract(u16 instruction, Mnemonic& mnemonic);
void DisassembleMoveShiftedRegister(u16 instruction, Mnemonic& mnemonic);
}  // namespace cpu::thumb
