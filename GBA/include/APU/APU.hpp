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
    MemReadData8 ReadReg8(Address addr) override;
    MemReadData16 ReadReg16(Address addr) override;
    MemReadData32 ReadReg32(Address addr) override;

    CpuCycles WriteReg8(Address addr, u8 val) override;
    CpuCycles WriteReg16(Address addr, u16 val) override;
    CpuCycles WriteReg32(Address addr, u32 val) override;

private:
    std::array<std::byte, 0x48> registers_;
};
}  // namespace audio
