#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <unordered_set>
#include <GBA/include/Debug/DebugTypes.hpp>
#include <GBA/include/Keypad/Registers.hpp>
#include <GBA/include/Utilities/Types.hpp>

namespace fs = std::filesystem;

namespace gba_api
{
/// @brief Initialize a GBA instance if one doesn't already exist.
/// @param biosPath Path to BIOS ROM file to load.
/// @param romPath Path to GBA ROM file to load.
/// @param saveDir Path to directory to store save files and save states.
/// @param vBlankCallback Function to be called whenever the GBA enters VBlank.
/// @param breakpointCallback Function to be called whenever the GBA encounters a breakpoint set in the CPU debugger.
void InitializeGBA(fs::path biosPath,
                   fs::path romPath,
                   fs::path saveDir,
                   std::function<void()> vBlankCallback,
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

/// @brief Set the CPU clock speed.
/// @param clockSpeed New CPU clock speed in Hz.
void SetCpuClockSpeed(u32 clockSpeed);

///---------------------------------------------------------------------------------------------------------------------------------
/// Debug
///---------------------------------------------------------------------------------------------------------------------------------

/// @brief Run the emulator for a single CPU instruction.
void StepCPU();

/// @brief Run the emulator until the next time it hits VBlank.
void StepFrame();

/// @brief Get debug info needed to draw a fully isolated background layer.
/// @param debugInfo Reference to background debug info to be populated.
/// @param bgIndex Index of background to display in debugger.
void GetBgDebugInfo(debug::BackgroundDebugInfo& debugInfo, u8 bgIndex);

/// @brief Get debug info needed to display sprites in sprite debugger window.
/// @param sprites Reference to sprites to update with current OAM data.
/// @param regTransforms Apply transforms (horizontal and vertical flip) to regular sprites.
/// @param affTransforms Apply transforms (use affine matrix) to affine sprites.
void GetSpriteDebugInfo(debug::SpriteDebugInfo& sprites, bool regTransforms, bool affTransforms);

/// @brief Get debug info to be shown in the CPU Debugger.
/// @return CPU debug info.
debug::CpuDebugInfo GetCpuDebugInfo();

/// @brief Disassemble an ARM instruction into its human-readable mnemonic.
/// @param instruction Raw 32-bit ARM instruction code.
/// @return Disassembled instruction.
debug::Mnemonic const& DisassembleArmInstruction(u32 instruction);

/// @brief Disassemble a THUMB instruction into its human-readable mnemonic.
/// @param instruction Raw 16-bit THUMB instruction code.
/// @return Disassembled instruction.
debug::Mnemonic const& DisassembleThumbInstruction(u32 instruction);

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

/// @brief Get the value of an I/O register.
/// @param addr Address of register.
/// @param size Size of the register in bytes.
/// @return Current value of specified register.
u32 DebugReadRegister(u32 addr, u8 size);
}  // namespace gui
