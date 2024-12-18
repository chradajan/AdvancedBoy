#pragma once

#include <array>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <vector>
#include <GBA/include/PPU/FrameBuffer.hpp>
#include <GBA/include/PPU/Registers.hpp>
#include <GBA/include/PPU/VramViews.hpp>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>
#include <GBA/include/Utilities/Types.hpp>

namespace debug { class PPUDebugger; }

namespace graphics
{
/// @brief Pixel Processing Unit.
class PPU
{
public:
    PPU() = delete;
    PPU(PPU const&) = delete;
    PPU& operator=(PPU const&) = delete;
    PPU(PPU&&) = delete;
    PPU& operator=(PPU&&) = delete;

    /// @brief Initialize the PPU.
    /// @param scheduler Reference to event scheduler to post PPU state change events to.
    /// @param systemControl Reference to system control to post PPU interrupts to.
    explicit PPU(EventScheduler& scheduler, SystemControl& systemControl);

    /// @brief Get a pointer to the pixel data of the most recently completed frame.
    /// @return Pointer to raw pixel data.
    uchar* GetRawFrameBuffer() { return frameBuffer_.GetRawFrameBuffer(); }

    /// @brief Get the number of frames that have been generated since the last check. Reset the counter.
    /// @return Number of times the PPU has entered VBlank since last check.
    int GetAndResetFPSCounter() { int counter = fpsCounter_; fpsCounter_ = 0; return counter; }

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Bus functionality
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Read an address in PRAM.
    /// @param addr Address to read from.
    /// @param length Memory access size of the read.
    /// @return Number of cycles taken to read, value returned from the read, and whether it was an open-bus read.
    MemReadData ReadPRAM(u32 addr, AccessSize length);

    /// @brief Write to an address in PRAM.
    /// @param addr Address to write to.
    /// @param val Value to write.
    /// @param length Memory access size of the write.
    /// @return Number of cycles taken to write.
    int WritePRAM(u32 addr, u32 val, AccessSize length);

    /// @brief Read an address in OAM.
    /// @param addr Address to read from.
    /// @param length Memory access size of the read.
    /// @return Number of cycles taken to read, value returned from the read, and whether it was an open-bus read.
    MemReadData ReadOAM(u32 addr, AccessSize length);

    /// @brief Write to an address in OAM.
    /// @param addr Address to write to.
    /// @param val Value to write.
    /// @param length Memory access size of the write.
    /// @return Number of cycles taken to write.
    int WriteOAM(u32 addr, u32 val, AccessSize length);

    /// @brief Read an address in VRAM.
    /// @param addr Address to read from.
    /// @param length Memory access size of the read.
    /// @return Number of cycles taken to read, value returned from the read, and whether it was an open-bus read.
    MemReadData ReadVRAM(u32 addr, AccessSize length);

    /// @brief Write to an address in VRAM.
    /// @param addr Address to write to.
    /// @param val Value to write.
    /// @param length Memory access size of the write.
    /// @return Number of cycles taken to write.
    int WriteVRAM(u32 addr, u32 val, AccessSize length);

    /// @brief Read an address mapped to PPU registers.
    /// @param addr Address of PPU register(s).
    /// @param length Memory access size of the read.
    /// @return Number of cycles taken to read, value of the requested register(s), and whether it was an open-bus read.
    MemReadData ReadReg(u32 addr, AccessSize length);

    /// @brief Write to an address mapped to PPU registers.
    /// @param addr Address of PPU register(s).
    /// @param val Value to write to register(s).
    /// @param length Memory access size of the write.
    /// @return Number of cycles taken to write.
    int WriteReg(u32 addr, u32 val, AccessSize length);

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Event Handlers
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief HBlank event handler.
    /// @param extraCycles Cycles since this event was scheduled to execute.
    void HBlank(int extraCycles);

    /// @brief VBlank event handler.
    /// @param extraCycles Cycles since this event was scheduled to execute.
    void VBlank(int extraCycles);

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Register access/updates
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Get the current value of VCOUNT.
    /// @return Current scanline.
    u8 GetVCOUNT() const { return MemCpyInit<u8>(&registers_[VCOUNT::INDEX]); }

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Save States
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Write data to save state file.
    /// @param saveState Save state stream to write to.
    void Serialize(std::ofstream& saveState) const;

    /// @brief Load data from save state file.
    /// @param saveState Save state stream to read from.
    void Deserialize(std::ifstream& saveState);

private:
    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Register access/updates
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Get the current value of the LCD Control register.
    /// @return Current DISPCNT value.
    DISPCNT GetDISPCNT() const { return MemCpyInit<DISPCNT>(&registers_[DISPCNT::INDEX]); }

    /// @brief Get the current value of the LCD Status register.
    /// @return Current DISPSTAT value.
    DISPSTAT GetDISPSTAT() const { return MemCpyInit<DISPSTAT>(&registers_[DISPSTAT::INDEX]); }

    /// @brief Update the value of the DISPSTAT register.
    /// @param reg New value of DISPSTAT.
    void SetDISPSTAT(DISPSTAT reg) { std::memcpy(&registers_[DISPSTAT::INDEX], &reg, sizeof(DISPSTAT)); }

    /// @brief Update the value of the VCOUNT register.
    /// @param reg New value of VCOUNT.
    void SetVCOUNT(u8 reg) { std::memcpy(&registers_[VCOUNT::INDEX], &reg, sizeof(u8)); }

    /// @brief Get a background control register.
    /// @param bg Which background to get control register for.
    /// @return Background control register.
    BGCNT GetBGCNT(u8 bg) const { return MemCpyInit<BGCNT>(&registers_[BGCNT::INDEX + (2 * bg)]); }

    /// @brief Set internal register corresponding to BG2X.
    void SetBG2RefX();

    /// @brief Set internal register corresponding to BG2Y.
    void SetBG2RefY();

    /// @brief Set internal register corresponding to BG3X.
    void SetBG3RefX();

    /// @brief Set internal register corresponding to BG3Y.
    void SetBG3RefY();

    /// @brief Increment BG2 and BG3 reference points after a scanline is rendered.
    void IncrementAffineBackgroundReferencePoints();

    /// @brief Handle writes to the DISPSTAT and VCOUNT registers to avoid writing to read only bits.
    /// @param addr Address of register(s) being written.
    /// @param val Value to write to register(s).
    /// @param length Memory access size of the write.
    void WriteDispstatVcount(u32 addr, u32 val, AccessSize length);

    /// @brief Check for a VCOUNTER match. Update DISPSTAT and request interrupt upon match.
    void CheckVCountSetting();

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// PRAM access
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Get a background color in 16/16 color mode.
    /// @param palette Index of palette.
    /// @param index Index of color within palette.
    /// @return BGR555 value.
    u16 GetBgColor(u8 palette, u8 index) const { return MemCpyInit<u16>(&PRAM_.at(((palette * 16) + index) * sizeof(u16))); }

    /// @brief Get a background color in 256/1 color mode.
    /// @param index Index of color.
    /// @return BGR555 value.
    u16 GetBgColor(u8 index) const { return MemCpyInit<u16>(&PRAM_.at(index * sizeof(u16))); }

    /// @brief Get a sprite color in 16/16 color mode.
    /// @param palette Index of palette.
    /// @param index Index of color within palette.
    /// @return BGR555 value.
    u16 GetSpriteColor(u8 palette, u8 index) const {
        return MemCpyInit<u16>(&PRAM_.at(512 + (((palette * 16) + index) * sizeof(u16))));
    }

    /// @brief Get a sprite color in 256/1 color mode.
    /// @param index Index of color.
    /// @return BGR555 value.
    u16 GetSpriteColor(u8 index) const { return MemCpyInit<u16>(&PRAM_.at(512 + (index * sizeof(u16)))); }

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Event Handlers
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief VDraw event handler.
    /// @param extraCycles Cycles since this event was scheduled to execute.
    void VDraw(int extraCycles);

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Window control
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Determine whether window 0 and window 1 are active on the current scanline.
    void SetNonObjWindowEnabled();

    /// @brief Apply window settings to pixels within a window on the current scanline.
    /// @param leftEdge X1 - Left edge of window (inclusive).
    /// @param rightEdge X2 - Right edge of window (exclusive).
    /// @param settings Settings for inside this window region.
    void ConfigureNonObjWindow(u8 leftEdge, u8 rightEdge, WindowSettings const& settings);

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Rendering
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Evaluate sprites, window, and background layers on the current scanline.
    void EvaluateScanline();

    /// @brief Render background pixels in mode 0.
    void RenderMode0Scanline();

    /// @brief Render background pixels in mode 1.
    void RenderMode1Scanline();

    /// @brief Render background pixels in mode 2.
    void RenderMode2Scanline();

    /// @brief Render background pixels in mode 3.
    void RenderMode3Scanline();

    /// @brief Render background pixels in mode 4.
    void RenderMode4Scanline();

    /// @brief Render a regular tiled background scanline.
    /// @param bgcnt BGCNT register for the background to draw.
    /// @param bgIndex BG index (0-3).
    /// @param xOffset X-offset of the background to draw.
    /// @param yOffset Y-offset of the background to draw.
    void RenderRegularTiledBackgroundScanline(BGCNT bgcnt, u8 bgIndex, u16 xOffset, u16 yOffset);

    /// @brief Render a regular tiled text background scanline that uses 4bpp colors.
    /// @param bgcnt Control register of specified background.
    /// @param bgIndex Which background to render.
    /// @param x X-coordinate within background map.
    /// @param y Y-Coordinate within background map.
    /// @param width Width of map.
    void RenderRegular4bppBackground(BGCNT bgcnt, u8 bgIndex, u16 x, u16 y, u16 width);

    /// @brief Render a regular tiled text background scanline that uses 8bpp colors.
    /// @param bgcnt Control register of specified background.
    /// @param bgIndex Which background to render.
    /// @param x X-coordinate within background map.
    /// @param y Y-Coordinate within background map.
    /// @param width Width of map.
    void RenderRegular8bppBackground(BGCNT bgcnt, u8 bgIndex, u16 x, u16 y, u16 width);

    /// @brief Render a tiled affined background scanline.
    /// @param bgcnt Control register of specified background.
    /// @param bgIndex Which background to render.
    /// @param x Reference point x-coordinate.
    /// @param y Reference point y-coordinate.
    /// @param dx Amount to increment x-coordinate by after each pixel.
    /// @param dy Amount to increment y-coordinate by after each pixel.
    void RenderAffineTiledBackgroundScanline(BGCNT bgcnt, u8 bgIndex, i32 x, i32 y, i16 dx, i16 dy);

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Sprites
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Render sprites and mix with background.
    /// @param windowSettingsPtr Pointer to OBJ window settings, or nullptr if rendering a visible sprites.
    void EvaluateOAM(WindowSettings* windowSettingsPtr = nullptr);

    /// @brief Render a regular sprite. Handles 4bpp/8bpp and 1D/2D mapping.
    /// @param oneDim Char block mapping mode. True for 1D, false for 2D.
    /// @param x X-coordinate of top left corner of sprite.
    /// @param y Y-coordinate of top left corner of sprite.
    /// @param width Width of sprite in pixels.
    /// @param height Height of sprite in pixels.
    /// @param entry Reference to OAM entry for sprite.
    /// @param windowSettingsPtr Pointer to OBJ window settings, or nullptr if rendering a visible sprite.
    void RenderRegSprite(bool oneDim, i16 x, i16 y, u8 width, u8 height, OamEntry const& entry, WindowSettings* windowSettingsPtr);

    /// @brief Render an affine sprite. Handles 4bpp/8bpp and 1D/2D mapping.
    /// @param oneDim Char block mapping mode. True for 1D, false for 2D.
    /// @param x X-coordinate of top left corner of sprite.
    /// @param y Y-coordinate of top left corner of sprite.
    /// @param width Width of sprite in pixels.
    /// @param height Height of sprite in pixels.
    /// @param entry Reference to OAM entry for sprite.
    /// @param windowSettingsPtr Pointer to OBJ window settings, or nullptr if rendering a visible sprite.
    void RenderAffSprite(bool oneDim, i16 x, i16 y, u8 width, u8 height, OamEntry const& entry, WindowSettings* windowSettingsPtr);

    /// @brief Evaluate whether a sprite pixel should be considered for rendering or for the OBJ window.
    /// @param dot Dot location of the pixel.
    /// @param color BGR555 value of the pixel.
    /// @param priority Priority (0-3) of this pixel.
    /// @param transparent Whether the pixel is transparent.
    /// @param semiTransparent Whether the pixel is semi-transparent.
    /// @param windowSettingsPtr Pointer to window settings to be applied. If nullptr, then consider pixel for rendering instead.
    void PushSpritePixel(u8 dot, u16 color, u8 priority, bool transparent, bool semiTransparent, WindowSettings* windowSettingsPtr);

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Member data
    ///-----------------------------------------------------------------------------------------------------------------------------

    // Window
    bool window0EnabledOnScanline_;
    bool window1EnabledOnScanline_;

    // Affine BG internal reference registers
    i32 bg2RefX_;
    i32 bg2RefY_;
    i32 bg3RefX_;
    i32 bg3RefY_;

    // Memory
    std::array<std::byte, PRAM_SIZE> PRAM_;
    alignas(OamEntry) std::array<std::byte, OAM_SIZE> OAM_;
    std::array<std::byte, VRAM_SIZE> VRAM_;

    // Registers
    std::array<std::byte, 0x58> registers_;

    // Frame buffer
    FrameBuffer frameBuffer_;
    int fpsCounter_;

    // External components
    EventScheduler& scheduler_;
    SystemControl& systemControl_;

    // VRAM views
    friend class debug::PPUDebugger;
    friend class RegularScreenBlockScanlineView;
};
}  // namespace graphics
