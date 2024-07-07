#pragma once

#include <array>
#include <cstddef>
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

private:
    // Memory
    std::array<std::byte,  1 * KiB> PRAM_;
    std::array<std::byte,  1 * KiB> OAM_;
    std::array<std::byte, 96 * KiB> VRAM_;

    // Registers
    std::array<std::byte, 0x58> registers_;

    // External components
    EventScheduler& scheduler_;
    SystemControl& systemControl_;
};
}  // namespace graphics
