#pragma once

#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/Types/DebugTypes.hpp>
#include <GBA/include/Types/Types.hpp>

namespace cpu::arm
{
using namespace debug::cpu;

/// @brief Convert ARM condition code to its mnemonic.
/// @param cond 4-bit ARM condition code.
/// @return Condition mnemonic.
std::string DecodeCondition(u8 cond);

/// @brief Disassemble an ARM instruction into its human-readable mnemonic.
/// @param instruction Raw 32-bit ARM instruction code.
/// @param addr Address of the instruction.
/// @return Disassembled instruction.
DisassembledInstruction DisassembleInstruction(u32 instruction, u32 addr);

///---------------------------------------------------------------------------------------------------------------------------------
/// @brief Functions to convert 32-bit ARM instructions to their mnemonics and arguments.
///---------------------------------------------------------------------------------------------------------------------------------

void DisassembleBranchAndExchange(u32 instruction, DisassembledInstruction& disassembly);
void DisassembleBlockDataTransfer(u32 instruction, DisassembledInstruction& disassembly);
void DisassembleBranch(u32 instruction, u32 pc, DisassembledInstruction& disassembly);
void DisassembleSoftwareInterrupt(u32 instruction, DisassembledInstruction& disassembly);
void DisassembleUndefined(u32 instruction, DisassembledInstruction& disassembly);
void DisassembleSingleDataTransfer(u32 instruction, DisassembledInstruction& disassembly);
void DisassembleSingleDataSwap(u32 instruction, DisassembledInstruction& disassembly);
void DisassembleMultiply(u32 instruction, DisassembledInstruction& disassembly);
void DisassembleMultiplyLong(u32 instruction, DisassembledInstruction& disassembly);
void DisassembleHalfwordDataTransfer(u32 instruction, DisassembledInstruction& disassembly);
void DisassemblePSRTransferMRS(u32 instruction, DisassembledInstruction& disassembly);
void DisassemblePSRTransferMSR(u32 instruction, DisassembledInstruction& disassembly);
void DisassembleDataProcessing(u32 instruction, DisassembledInstruction& disassembly);
}  // namespace cpu::arm
