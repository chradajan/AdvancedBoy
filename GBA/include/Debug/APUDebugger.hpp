#pragma once

#include <GBA/include/APU/APU.hpp>
#include <GBA/include/Utilities/Types.hpp>

namespace debug
{
class APUDebugger
{
public:
    APUDebugger() = delete;
    APUDebugger(APUDebugger const&) = delete;
    APUDebugger& operator=(APUDebugger const&) = delete;
    APUDebugger(APUDebugger&&) = delete;
    APUDebugger& operator=(APUDebugger&&) = delete;

    /// @brief Initialize a debugger for the APU.
    /// @param apu Reference to APU.
    APUDebugger(audio::APU const& apu) : apu_(apu) {}

    /// @brief Get the value of an APU register.
    /// @param addr Address of register.
    /// @param length Memory access size of the read.
    /// @return Current value of specified register.
    u32 ReadRegister(u32 addr, AccessSize length) const;

private:
    audio::APU const& apu_;
};
}  // namespace debug
