#pragma once

#include <array>
#include <cstddef>
#include <GBA/include/Memory/RegisterComponentBase.hpp>
#include <GBA/include/Types.hpp>

namespace audio
{
/// @brief Audio Processing Unit.
class APU : public RegisterComponentBase
{
public:
    /// @brief Read an APU register.
    MemReadData ReadReg(Address addr, AccessSize length) override;

    /// @brief Write an APU register.
    CpuCycles WriteReg(Address addr, u32 val, AccessSize length) override;

private:
    std::array<std::byte, 0x48> registers_;
};
}  // namespace audio
