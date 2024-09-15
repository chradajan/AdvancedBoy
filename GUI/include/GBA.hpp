#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <unordered_set>
#include <GBA/include/Keypad/Registers.hpp>
#include <GBA/include/Types/DebugTypes.hpp>
#include <GBA/include/Types/Types.hpp>

namespace fs = std::filesystem;

namespace gba_api
{
/// @brief Initialize a GBA instance if one doesn't already exist.
/// @param biosPath Path to BIOS ROM file to load.
/// @param romPath Path to GBA ROM file to load.
/// @param logDir Path to directory to store logs. Pass an empty path to disable logging.
/// @param vBlankCallback Function to be called whenever the GBA enters VBlank.
/// @param breakpointCallback Function to be called whenever the GBA encounters a breakpoint set in the CPU debugger.
void InitializeGBA(fs::path biosPath,
                   fs::path romPath,
                   fs::path logDir,
                   std::function<void(int)> vBlankCallback,
                   std::function<void()> breakpointCallback);

/// @brief Delete the current GBA instance if it exists.
void PowerOff();

/// @brief Run the GBA main loop until its internal audio sample buffer is full.
void RunEmulationLoop();

/// @brief Drain any existing audio samples into a buffer to be played back.
/// @param stream Pointer to buffer where samples should be transferred to.
/// @param len Size of buffer in bytes.
void FillAudioBuffer(u8* stream, size_t len);

/// @brief Get a pointer to the most recently completed frame.
/// @return Pointer to frame buffer data.
uchar* GetFrameBuffer();

/// @brief Get FPS counter from PPU.
/// @return Number of times the PPU has entered VBlank since last check.
int GetFPSCounter();

/// @brief Get the title of the ROM currently running.
/// @return Current ROM title.
std::string GetTitle();

/// @brief Update the KEYINPUT register based on current user input.
/// @param keyinput KEYINPUT value.
void UpdateKeypad(KEYINPUT keyinput);

///---------------------------------------------------------------------------------------------------------------------------------
/// Debug
///---------------------------------------------------------------------------------------------------------------------------------

/// @brief Pre-disassemble all BIOS and ROM code as both ARM and THUMB instructions.
void RunDisassembler();

/// @brief Run the emulator for a single CPU instruction.
void SingleStep();

/// @brief Get debug info needed to draw a fully isolated background layer.
/// @param bgIndex Index of background to display in debugger.
/// @return Debug info needed to display a background layer.
debug::graphics::BackgroundDebugInfo GetBgDebugInfo(u8 bgIndex);

/// @brief Get debug info to be shown in the CPU Debugger.
/// @return CPU debug info.
debug::cpu::CpuDebugInfo GetCpuDebugInfo();

/// @brief Disassemble an ARM instruction into its human-readable mnemonic.
/// @param instruction Raw 32-bit ARM instruction code.
/// @return Disassembled instruction.
debug::cpu::Mnemonic const& DisassembleArmInstruction(u32 instruction);

/// @brief Disassemble a THUMB instruction into its human-readable mnemonic.
/// @param instruction Raw 16-bit THUMB instruction code.
/// @return Disassembled instruction.
debug::cpu::Mnemonic const& DisassembleThumbInstruction(u32 instruction);

/// @brief Add a breakpoint at a specified address. CPU execution will stop when this matches the address of the next instruction to
///        be executed by the CPU.
/// @param breakpoint Address to set breakpoint at.
void SetBreakpoint(u32 breakpoint);

/// @brief Remove an address from the current list of breakpoints.
/// @param breakpoint Address to set breakpoint at.
void RemoveBreakpoint(u32 breakpoint);

/// @brief Get the list of breakpoints currently set.
/// @return An unordered set of all current breakpoints.
std::unordered_set<u32> const& GetBreakpoints();
}  // namespace gui
