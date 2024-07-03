#pragma once

#include <array>
#include <cstddef>
#include <GBA/include/Memory/RegisterComponentBase.hpp>
#include <GBA/include/Types.hpp>

namespace timers
{
/// @brief Manager class for system timers.
class TimerManager : public RegisterComponentBase
{
public:
    /// @brief Read a Timer register.
    MemReadData ReadReg(Address addr, AccessSize length) override;

    /// @brief Write a Timer register.
    CpuCycles WriteReg(Address addr, u32 val, AccessSize length) override;

private:
    std::array<std::byte, 0x10> registers_;
};
}  // namespace timers
