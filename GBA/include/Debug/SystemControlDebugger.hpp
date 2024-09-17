#pragma once

#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Utilities/Types.hpp>

namespace debug
{
class SystemControlDebugger
{
public:
    SystemControlDebugger() = delete;
    SystemControlDebugger(SystemControlDebugger const&) = delete;
    SystemControlDebugger& operator=(SystemControlDebugger const&) = delete;
    SystemControlDebugger(SystemControlDebugger&&) = delete;
    SystemControlDebugger& operator=(SystemControlDebugger&&) = delete;

    /// @brief Initialize a debugger for System Control.
    /// @param systemControl Reference to System Control.
    SystemControlDebugger(SystemControl const& systemControl) : systemControl_(systemControl) {}

    /// @brief Get the value of a System Control register.
    /// @param addr Address of register.
    /// @param length Memory access size of the read.
    /// @return Current value of specified register.
    u32 ReadRegister(u32 addr, AccessSize length) const;

private:
    SystemControl const& systemControl_;
};
}  // namespace debug
