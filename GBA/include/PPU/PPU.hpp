#pragma once

#include <array>
#include <cstddef>
#include <cstring>
#include <GBA/include/PPU/FrameBuffer.hpp>
#include <GBA/include/PPU/Registers.hpp>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Types.hpp>

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

private:
    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Register access/updates
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Get the current value of the LCD Control register.
    /// @return Current DISPCNT value.
    DISPCNT GetDISPCNT() const { DISPCNT reg; std::memcpy(&reg, &registers_[DISPCNT::INDEX], sizeof(DISPCNT)); return reg; }

    /// @brief Get the current value of the LCD Status register.
    /// @return Current DISPSTAT value.
    DISPSTAT GetDISPSTAT() const { DISPSTAT reg; std::memcpy(&reg, &registers_[DISPSTAT::INDEX], sizeof(DISPSTAT)); return reg; }

    /// @brief Update the value of the DISPSTAT register.
    /// @param reg New value of DISPSTAT.
    void SetDISPSTAT(DISPSTAT reg) { std::memcpy(&registers_[DISPSTAT::INDEX], &reg, sizeof(DISPSTAT)); }

    /// @brief Get the current value of VCOUNT.
    /// @return Current scanline.
    u8 GetVCOUNT() const { u8 reg; std::memcpy(&reg, &registers_[VCOUNT::INDEX], sizeof(u8)); return reg; }

    /// @brief Update the value of the VCOUNT register.
    /// @param reg New value of VCOUNT.
    void SetVCOUNT(u8 reg) { std::memcpy(&registers_[VCOUNT::INDEX], &reg, sizeof(u8)); }

    /// @brief Get a background control register.
    /// @param bg Which background to get control register for.
    /// @return Background control register.
    BGCNT GetBGCNT(u8 bg) const { BGCNT reg; std::memcpy(&reg, &registers_[BGCNT::INDEX + (2 * bg)], sizeof(BGCNT)); return reg; }

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
    u16 GetBgColor(u8 palette, u8 index) const {
        u16 val; std::memcpy(&val, &PRAM_.at(((palette * 16) + index) * 2), sizeof(u16)); return val;
    }

    /// @brief Get a background color in 256/1 color mode.
    /// @param index Index of color.
    /// @return BGR555 value.
    u16 GetBgColor(u8 index) const { u16 val; std::memcpy(&val, &PRAM_.at(index * sizeof(u16)), sizeof(u16)); return val; }

    /// @brief Get a sprite color in 16/16 color mode.
    /// @param palette Index of palette.
    /// @param index Index of color within palette.
    /// @return BGR555 value.
    u16 GetSpriteColor(u8 palette, u8 index) const {
        u16 val; std::memcpy(&val, &PRAM_.at(512 + (((palette * 16) + index) * sizeof(u16))), sizeof(u16)); return val;
    }

    /// @brief Get a sprite color in 256/1 color mode.
    /// @param index Index of color.
    /// @return BGR555 value.
    u16 GetSpriteColor(u8 index) const {
        u16 val; std::memcpy(&val, &PRAM_.at(512 + (index * sizeof(u16))), sizeof(u16)); return val;
    }

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
    std::array<std::byte,  1 * KiB> PRAM_;
    std::array<std::byte,  1 * KiB> OAM_;
    std::array<std::byte, 96 * KiB> VRAM_;

    // Registers
    std::array<std::byte, 0x58> registers_;

    // Frame buffer
    FrameBuffer frameBuffer_;
    int fpsCounter_;

    // External components
    EventScheduler& scheduler_;
    SystemControl& systemControl_;

    // VRAM views
    friend class BackgroundCharBlockView;
    friend class RegularScreenBlockScanlineView;
};
}  // namespace graphics
