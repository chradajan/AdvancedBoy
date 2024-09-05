#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <GBA/include/Keypad/Registers.hpp>
#include <GBA/include/Types/DebugTypes.hpp>
#include <GBA/include/Types/Types.hpp>

namespace fs = std::filesystem;

namespace gui
{
/// @brief Initialize a GBA instance if one doesn't already exist.
/// @param biosPath Path to BIOS ROM file to load.
/// @param romPath Path to GBA ROM file to load.
/// @param logDir Path to directory to store logs. Pass an empty path to disable logging.
/// @param vBlankCallback Function to be called whenever the GBA enters VBlank.
void InitializeGBA(fs::path biosPath, fs::path romPath, fs::path logDir, std::function<void(int)> vBlankCallback);

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

/// @brief Get debug info needed to draw a fully isolated background layer.
/// @param bgIndex Index of background to display in debugger.
/// @return Debug info needed to display a background layer.
debug::graphics::BackgroundDebugInfo GetBgDebugInfo(u8 bgIndex);
}  // namespace gui
