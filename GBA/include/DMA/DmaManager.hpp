#pragma once

#include <array>
#include <cstddef>
#include <utility>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Types.hpp>
#include <GBA/include/Utilities/Functor.hpp>

class GameBoyAdvance;

namespace dma
{
/// @brief DMA channel manager.
class DmaManager
{
    using ReadMemCallback = MemberFunctor<std::pair<u32, CpuCycles> (GameBoyAdvance::*)(Address, AccessSize)>;
    using WriteMemCallback = MemberFunctor<CpuCycles (GameBoyAdvance::*)(Address, u32, AccessSize)>;

public:
    DmaManager() = delete;
    DmaManager(DmaManager const&) = delete;
    DmaManager& operator=(DmaManager const&) = delete;
    DmaManager(DmaManager&&) = delete;
    DmaManager& operator=(DmaManager&&) = delete;

    /// @brief Initialize the DMA manager and the four DMA channels.
    /// @param readMem Callback function to access bus read functionality.
    /// @param writeMem Callback function to access bus write functionality.
    /// @param systemControl Reference to system control to post DMA interrupts to.
    explicit DmaManager(ReadMemCallback readMem, WriteMemCallback writeMem, SystemControl& systemControl);

    /// @brief Read an address mapped to DMA registers.
    /// @param addr Address of DMA register(s).
    /// @param length Memory access size of the read.
    /// @return Number of cycles taken to read, value of the requested register(s), and whether it was an open-bus read.
    MemReadData ReadReg(Address addr, AccessSize length);

    /// @brief Write to an address mapped to DMA registers.
    /// @param addr Address of DMA register(s).
    /// @param val Value to write to register(s).
    /// @param length Memory access size of the write.
    /// @return Number of cycles taken to write.
    CpuCycles WriteReg(Address addr, u32 val, AccessSize length);

private:
    ReadMemCallback ReadMemory;
    WriteMemCallback WriteMemory;

    std::array<std::byte, 0x30> registers_;

    // External components
    SystemControl& systemControl_;
};
}  // namespace dma
