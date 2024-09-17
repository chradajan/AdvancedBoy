#pragma once

#include <GBA/include/Debug/CPUDebugger.hpp>
#include <GBA/include/Debug/PPUDebugger.hpp>
#include <GBA/include/GameBoyAdvance.hpp>
#include <GBA/include/PPU/PPU.hpp>
#include <GBA/include/Utilities/Types.hpp>

class GameBoyAdvance;

namespace debug
{
class GameBoyAdvanceDebugger
{
public:
    GameBoyAdvanceDebugger() = delete;
    GameBoyAdvanceDebugger(GameBoyAdvanceDebugger const&) = delete;
    GameBoyAdvanceDebugger& operator=(GameBoyAdvanceDebugger const&) = delete;
    GameBoyAdvanceDebugger(GameBoyAdvanceDebugger&&) = delete;
    GameBoyAdvanceDebugger& operator=(GameBoyAdvanceDebugger&&) = delete;

    /// @brief Initialize a debugger for a Game Boy Advance.
    /// @param gba Reference to GBA.
    GameBoyAdvanceDebugger(GameBoyAdvance const& gba);

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Memory
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Get direct access to a block of memory for debug purposes.
    /// @param addr Get block of data that addr is contained within.
    /// @return Debug mem access.
    DebugMemAccess GetDebugMemAccess(u32 addr) const;

    /// @brief Get the value of an I/O register for debugging purposes.
    /// @param addr Address of register.
    /// @param length Memory access size of the read.
    /// @return Current value of specified register.
    u32 ReadRegister(u32 addr, AccessSize length) const;

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// CPU
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Get debug info to be shown in the CPU Debugger.
    /// @return CPU debug info.
    CpuDebugInfo GetCpuDebugInfo() const;

    /// @brief Disassemble an ARM instruction into its human-readable mnemonic.
    /// @param instruction Raw 32-bit ARM instruction code.
    /// @return Disassembled instruction.
    Mnemonic const& DisassembleArmInstruction(u32 instruction) { return cpuDebugger_.DisassembleArmInstruction(instruction); }

    /// @brief Disassemble a THUMB instruction into its human-readable mnemonic.
    /// @param instruction Raw 16-bit THUMB instruction code.
    /// @return Disassembled instruction.
    Mnemonic const& DisassembleThumbInstruction(u16 instruction) { return cpuDebugger_.DisassembleThumbInstruction(instruction); }

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// PPU
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Get debug info needed to draw a fully isolated background layer.
    /// @param bgIndex Index of background to display in debugger.
    /// @return Debug info needed to display a background layer.
    debug::BackgroundDebugInfo GetBgDebugInfo(u8 bgIndex) { return ppuDebugger_.GetBackgroundDebugInfo(bgIndex); }

private:
    GameBoyAdvance const& gba_;
    CPUDebugger cpuDebugger_;
    PPUDebugger ppuDebugger_;
};
}  // namespace debug
