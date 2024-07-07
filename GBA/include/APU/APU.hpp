#pragma once

#include <array>
#include <cstddef>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/Types.hpp>

namespace audio
{
/// @brief Audio Processing Unit.
class APU
{
public:
    APU() = delete;
    APU(APU const&) = delete;
    APU& operator=(APU const&) = delete;
    APU(APU&&) = delete;
    APU& operator=(APU&&) = delete;

    /// @brief Initialize the APU.
    /// @param scheduler Reference to event scheduler to post audio events to.
    explicit APU(EventScheduler& scheduler);

    /// @brief Read an address mapped to APU registers.
    /// @param addr Address of APU register(s).
    /// @param length Memory access size of the read.
    /// @return Number of cycles taken to read, value of the requested register(s), and whether it was an open-bus read.
    MemReadData ReadReg(Address addr, AccessSize length);

    /// @brief Write to an address mapped to APU registers.
    /// @param addr Address of APU register(s).
    /// @param val Value to write to register(s).
    /// @param length Memory access size of the write.
    /// @return Number of cycles taken to write.
    CpuCycles WriteReg(Address addr, u32 val, AccessSize length);

private:
    std::array<std::byte, 0x48> registers_;

    // External components
    EventScheduler& scheduler_;
};
}  // namespace audio
