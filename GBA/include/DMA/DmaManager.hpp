#pragma once

#include <array>
#include <cstddef>
#include <utility>
#include <GBA/include/DMA/DmaChannel.hpp>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Utilities/Functor.hpp>
#include <GBA/include/Utilities/Types.hpp>

class GameBoyAdvance;
namespace cartridge { class GamePak; }

namespace dma
{
/// @brief DMA channel manager.
class DmaManager
{
public:
    DmaManager() = delete;
    DmaManager(DmaManager const&) = delete;
    DmaManager& operator=(DmaManager const&) = delete;
    DmaManager(DmaManager&&) = delete;
    DmaManager& operator=(DmaManager&&) = delete;

    /// @brief Initialize the DMA manager and the four DMA channels.
    /// @param readMem Callback function to access bus read functionality.
    /// @param writeMem Callback function to access bus write functionality.
    /// @param scheduler Reference to event scheduler to handle timing of end of DMA transfers.
    /// @param systemControl Reference to system control to post DMA interrupts to.
    explicit DmaManager(ReadMemCallback readMem,
                        WriteMemCallback writeMem,
                        EventScheduler& scheduler,
                        SystemControl& systemControl);

    /// @brief Provide direct access to the GamePak for EEPROM DMA transfers.
    /// @param gamePakPtr Pointer to GamePak.
    void ConnectGamePak(cartridge::GamePak* gamePakPtr);

    /// @brief Read an address mapped to DMA registers.
    /// @param addr Address of DMA register(s).
    /// @param length Memory access size of the read.
    /// @return Number of cycles taken to read, value of the requested register(s), and whether it was an open-bus read.
    MemReadData ReadReg(u32 addr, AccessSize length);

    /// @brief Write to an address mapped to DMA registers.
    /// @param addr Address of DMA register(s).
    /// @param val Value to write to register(s).
    /// @param length Memory access size of the write.
    /// @return Number of cycles taken to write.
    int WriteReg(u32 addr, u32 val, AccessSize length);

    /// @brief Check if one of the DMA channels is currently in the middle of a transfer.
    /// @return Whether any DMA channel is running.
    bool DmaRunning() const { return active_; }

    /// @brief Check for any DMA channels set to execute upon VBlank.
    void CheckVBlank() { CheckSpecialTiming(vBlank_); }

    /// @brief Check for any DMA channels set to execute upon HBlank.
    void CheckHBlank() { CheckSpecialTiming(hBlank_); }

    /// @brief Check for any DMA channels set to refill FIFO A.
    void CheckFifoA() { CheckSpecialTiming(fifoA_); }

    /// @brief Check for any DMA channels set to refill FIFO B.
    void CheckFifoB() { CheckSpecialTiming(fifoB_); }

private:
    /// @brief Callback function to resume normal execution after a DMA transfer completes.
    void EndDma(int) { active_ = false; }

    /// @brief Handle event scheduling and interrupt requests after performing a DMA transfer.
    /// @param result Struct of relevant information returned by the DMA channel that just executed.
    void HandleDmaEvents(ExecuteResult result);

    /// @brief Check for any channels set to execute for a special event.
    /// @param enabledChannels Array of bools indicating whether each channel should execute.
    void CheckSpecialTiming(std::array<bool, 4>& enabledChannels);

    std::array<DmaChannel, 4> dmaChannels_;
    std::array<bool, 4> vBlank_;
    std::array<bool, 4> hBlank_;
    std::array<bool, 4> fifoA_;
    std::array<bool, 4> fifoB_;
    std::array<bool, 4> videoCapture_;
    bool active_;

    // External components
    EventScheduler& scheduler_;
    SystemControl& systemControl_;
};
}  // namespace dma
