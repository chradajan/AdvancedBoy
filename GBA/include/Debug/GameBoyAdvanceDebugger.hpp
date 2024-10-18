#pragma once

#include <GBA/include/Debug/APUDebugger.hpp>
#include <GBA/include/Debug/CPUDebugger.hpp>
#include <GBA/include/Debug/PPUDebugger.hpp>
#include <GBA/include/Debug/SystemControlDebugger.hpp>
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
    BackgroundDebugInfo GetBgDebugInfo(u8 bgIndex) const { return ppuDebugger_.GetBackgroundDebugInfo(bgIndex); }

    /// @brief Get debug info needed to display sprites in sprite debugger window.
    /// @param sprites Reference to sprites to update with current OAM data.
    /// @param regTransforms Apply transforms (horizontal and vertical flip) to regular sprites.
    /// @param affTransforms Apply transforms (use affine matrix) to affine sprites.
    void GetSpriteDebugInfo(SpriteDebugInfo& sprites, bool regTransforms, bool affTransforms) const
    {
        ppuDebugger_.GetSpriteDebugInfo(sprites, regTransforms, affTransforms);
    }

private:
    GameBoyAdvance const& gba_;
    APUDebugger apuDebugger_;
    CPUDebugger cpuDebugger_;
    PPUDebugger ppuDebugger_;
    SystemControlDebugger systemControlDebugger_;
};
}  // namespace debug
