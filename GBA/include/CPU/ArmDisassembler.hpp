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
/// @return Disassembled instruction.
Mnemonic DisassembleInstruction(u32 instruction);

///---------------------------------------------------------------------------------------------------------------------------------
/// @brief Functions to convert 32-bit ARM instructions to their mnemonics and arguments.
///---------------------------------------------------------------------------------------------------------------------------------

void DisassembleBranchAndExchange(u32 instruction, Mnemonic& mnemonic);
void DisassembleBlockDataTransfer(u32 instruction, Mnemonic& mnemonic);
void DisassembleBranch(u32 instruction, Mnemonic& mnemonic);
void DisassembleSoftwareInterrupt(u32 instruction, Mnemonic& mnemonic);
void DisassembleUndefined(u32 instruction, Mnemonic& mnemonic);
void DisassembleSingleDataTransfer(u32 instruction, Mnemonic& mnemonic);
void DisassembleSingleDataSwap(u32 instruction, Mnemonic& mnemonic);
void DisassembleMultiply(u32 instruction, Mnemonic& mnemonic);
void DisassembleMultiplyLong(u32 instruction, Mnemonic& mnemonic);
void DisassembleHalfwordDataTransfer(u32 instruction, Mnemonic& mnemonic);
void DisassemblePSRTransferMRS(u32 instruction, Mnemonic& mnemonic);
void DisassemblePSRTransferMSR(u32 instruction, Mnemonic& mnemonic);
void DisassembleDataProcessing(u32 instruction, Mnemonic& mnemonic);
}  // namespace cpu::arm
