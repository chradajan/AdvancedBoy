#pragma once

#include <array>
#include <span>
#include <GBA/include/Debug/DebugTypes.hpp>
#include <GBA/include/PPU/PPU.hpp>
#include <GBA/include/PPU/Registers.hpp>
#include <GBA/include/PPU/VramViews.hpp>
#include <GBA/include/Utilities/Types.hpp>

namespace debug
{
using namespace graphics;

class PPUDebugger
{
public:
    PPUDebugger() = delete;
    PPUDebugger(PPUDebugger const&) = delete;
    PPUDebugger& operator=(PPUDebugger const&) = delete;
    PPUDebugger(PPUDebugger&&) = delete;
    PPUDebugger& operator=(PPUDebugger&&) = delete;

    /// @brief Initialize a debugger for the PPU.
    /// @param ppu Reference to PPU.
    PPUDebugger(graphics::PPU const& ppu) : ppu_(ppu) {}

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Memory Access
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Return a span of PRAM for debug purposes.
    /// @return PRAM.
    std::span<const std::byte> GetPRAM() const { return ppu_.PRAM_; }

    /// @brief Return a span of OAM for debug purposes.
    /// @return OAM.
    std::span<const std::byte> GetOAM() const { return ppu_.OAM_; }

    /// @brief Return a span of VRAM for debug purposes.
    /// @return VRAM.
    std::span<const std::byte> GetVRAM() const { return ppu_.VRAM_; }

    /// @brief Get the value of a PPU register.
    /// @param addr Address of register.
    /// @param length Memory access size of the read.
    /// @return Current value of specified register.
    u32 ReadRegister(u32 addr, AccessSize length) const;

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Background Rendering
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Draw full background and get background info for debugger.
    /// @param bgIndex Index of background to display in debugger.
    /// @return Background debug info.
    BackgroundDebugInfo GetBackgroundDebugInfo(u8 bgIndex) const;

    /// @brief Get debug info for a regular tiled background.
    /// @param bgIndex Selected background index.
    /// @param bgcnt Selected background control register.
    /// @param debugInfo Reference to debug info to update.
    void RenderRegularBackground(u8 bgIndex, BGCNT bgcnt, BackgroundDebugInfo& debugInfo) const;

    /// @brief Helper function for rendering regular backgrounds. Draws a single screen block.
    /// @param bgcnt Selected background control register.
    /// @param screenBlockIndex Index of screen block to draw.
    /// @param bufferBaseIndex Index in debug buffer to start drawing to.
    /// @param debugInfo Reference to debug info to update.
    void RenderRegularScreenBlock(BGCNT bgcnt, u8 screenBlockIndex, u32 bufferBaseIndex, BackgroundDebugInfo& debugInfo) const;

    /// @brief Get debug info for an affine background.
    /// @param bgIndex Selected background index.
    /// @param bgcnt Selected background control register.
    /// @param debugInfo Reference to debug info to update.
    void RenderAffineBackground(u8 bgIndex, BGCNT bgcnt, BackgroundDebugInfo& debugInfo) const;

    /// @brief Get debug info for a bitmap background in mode 3.
    /// @param debugInfo Reference to debug info to update.
    void RenderMode3Background(BackgroundDebugInfo& debugInfo) const;

    /// @brief Get debug info for a bitmap background in mode 4.
    /// @param frameSelect Which bitmap frame to draw.
    /// @param debugInfo Reference to debug info to update.
    void RenderMode4Background(bool frameSelect, BackgroundDebugInfo& debugInfo) const;

    /// @brief Get debug info needed to display sprites in sprite debugger window.
    /// @param sprites Reference to sprites to update with current OAM data.
    /// @param regTransforms Apply transforms (horizontal and vertical flip) to regular sprites.
/// @param affTransforms Apply transforms (use affine matrix) to affine sprites.
    void GetSpriteDebugInfo(SpriteDebugInfo& sprites, bool regTransforms, bool affTransforms) const;

private:
    graphics::PPU const& ppu_;
};
}  // namespace graphics
