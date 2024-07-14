#pragma once

#include <filesystem>
#include <GBA/include/Types.hpp>

namespace fs = std::filesystem;

namespace gui
{
/// @brief Initialize a GBA instance if one doesn't already exist.
/// @param biosPath Path to BIOS ROM file to load.
/// @param romPath Path to GBA ROM file to load.
/// @param logDir Path to directory to store logs. Pass an empty path to disable logging.
void InitializeGBA(fs::path biosPath, fs::path romPath, fs::path logDir);

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
u8* GetFrameBuffer();
}  // namespace gui
