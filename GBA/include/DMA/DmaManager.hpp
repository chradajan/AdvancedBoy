#pragma once

#include <array>
#include <cstddef>
#include <GBA/include/Memory/RegisterComponentBase.hpp>
#include <GBA/include/Types.hpp>

namespace dma
{
/// @brief DMA channel manager.
class DmaManager : public RegisterComponentBase
{
public:
    /// @brief Read a DMA register.
    MemReadData ReadReg(Address addr, AccessSize length) override;

    /// @brief Write a DMA register.
    CpuCycles WriteReg(Address addr, u32 val, AccessSize length) override;

private:
    std::array<std::byte, 0x30> registers_;
};
}  // namespace dma
