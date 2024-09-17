#pragma once

#include <unordered_map>
#include <GBA/include/CPU/ARM7TDMI.hpp>

namespace debug
{
class CPUDebugger
{
public:
    CPUDebugger() = delete;
    CPUDebugger(CPUDebugger const&) = delete;
    CPUDebugger& operator=(CPUDebugger const&) = delete;
    CPUDebugger(CPUDebugger&&) = delete;
    CPUDebugger& operator=(CPUDebugger&&) = delete;

    /// @brief Initialize a debugger for the CPU.
    /// @param ppu Reference to CPU.
    CPUDebugger(cpu::ARM7TDMI const& cpu) : cpu_(cpu) {}

    /// @brief Get the current state of the ARM7TDMI registers.
    /// @param regState Reference to register state to populate.
    void PopulateCpuRegState(RegState& regState) const;

    /// @brief Disassemble an ARM instruction into its human-readable mnemonic.
    /// @param instruction Raw 32-bit ARM instruction code.
    /// @return Disassembled instruction.
    Mnemonic const& DisassembleArmInstruction(u32 instruction);

    /// @brief Disassemble a THUMB instruction into its human-readable mnemonic.
    /// @param instruction Raw 16-bit THUMB instruction code.
    /// @return Disassembled instruction.
    Mnemonic const& DisassembleThumbInstruction(u16 instruction);

private:
    cpu::ARM7TDMI const& cpu_;
    std::unordered_map<u32, Mnemonic> decodedArmInstructions_;
    std::unordered_map<u16, Mnemonic> decodedThumbInstructions_;
};
}  // debug
