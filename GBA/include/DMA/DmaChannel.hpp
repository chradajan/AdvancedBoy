#pragma once

#include <array>
#include <cstddef>
#include <cstring>
#include <optional>
#include <utility>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>
#include <GBA/include/Utilities/Functor.hpp>
#include <GBA/include/Utilities/Types.hpp>

class GameBoyAdvance;
namespace cartridge { class GamePak; }

namespace dma
{
using ReadMemCallback = MemberFunctor<std::pair<u32, int> (GameBoyAdvance::*)(u32, AccessSize)>;
using WriteMemCallback = MemberFunctor<int (GameBoyAdvance::*)(u32, u32, AccessSize)>;

/// @brief Transfer type options for a DMA channel.
enum class XferType
{
    NO_CHANGE,
    DISABLED,
    IMMEDIATE,
    VBLANK,
    HBLANK,
    FIFO_A,
    FIFO_B,
    VIDEO_CAPTURE
};

/// @brief Relevant information for DmaManager after a channel is executed.
struct ExecuteResult
{
    int cycles;
    bool enabled;
    std::optional<InterruptType> interrupt;
};

/// @brief Class representing a single DMA channel.
class DmaChannel
{
public:
    DmaChannel() = delete;
    DmaChannel(DmaChannel const&) = delete;
    DmaChannel& operator=(DmaChannel const&) = delete;
    DmaChannel(DmaChannel&&) = delete;
    DmaChannel& operator=(DmaChannel&&) = delete;

    /// @brief Initialize this DMA channel.
    /// @param index Which DMA channel this is.
    /// @param readMem Callback function to access bus read functionality.
    /// @param writeMem Callback function to access bus write functionality.
    explicit DmaChannel(u8 index, InterruptType interrupt, ReadMemCallback readMem, WriteMemCallback writeMem);

    /// @brief Read an address mapped to a DMA register.
    /// @param addr Address of register(s).
    /// @param length Memory access size of the read.
    /// @return Number of cycles taken to read, value of the requested register(s), and whether it was an open-bus read.
    MemReadData ReadReg(u32 addr, AccessSize length);

    /// @brief Write to an address mapped to a DMA register.
    /// @param addr Address of register(s).
    /// @param val Value to write to register(s).
    /// @param length Memory access size of the write.
    /// @return State of this channel's transfer type after the write.
    XferType WriteReg(u32 addr, u32 val, AccessSize length);

    /// @brief Provide direct access to the GamePak for EEPROM DMA transfers.
    /// @param gamePakPtr Pointer to GamePak.
    void ConnectGamePak(cartridge::GamePak* gamePakPtr) { gamePakPtr_ = gamePakPtr; }

    /// @brief Execute a DMA transfer with this channel's parameters.
    /// @return Number of cycles taken to perform the transfer, whether this channel is still enabled post transfer, and if this
    ///         channel is set to trigger an IRQ, the interrupt type corresponding to this channel.
    ExecuteResult Execute();

private:
    struct DMACNT
    {
        static constexpr size_t INDEX = 10;

        u16             : 5;
        u16 destAddrCnt : 2;
        u16 srcAddrCnt  : 2;
        u16 repeat      : 1;
        u16 xferType    : 1;
        u16 gamePakDrq  : 1;
        u16 timing      : 2;
        u16 irq         : 1;
        u16 enable      : 1;
    };

    static_assert(sizeof(DMACNT) == 2, "DMACNT must be 2 bytes");

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Register access/updates
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Get the current value of the DMA control register.
    /// @return Current DMACNT value.
    DMACNT GetDMACNT() const { return MemCpyInit<DMACNT>(&registers_[DMACNT::INDEX]); }

    /// @brief Update the value of the DMACNT register.
    /// @param reg New value of DMACNT.
    void SetDMACNT(DMACNT reg) { std::memcpy(&registers_[DMACNT::INDEX], &reg, sizeof(DMACNT)); }

    /// @brief Get the current value of the SAD register.
    /// @return Current SAD value.
    u32 GetSAD() const { return MemCpyInit<u32>(&registers_[0]); }

    /// @brief Get the current value of the DAD register.
    /// @return Current DAD value.
    u32 GetDAD() const { return MemCpyInit<u32>(&registers_[4]); }

    /// @brief Get the current value of the CNT register.
    /// @return Current CNT value.
    u16 GetWordCount() const { return MemCpyInit<u16>(&registers_[8]); }

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Transfer type determination
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Determine what kind of transfer this channel is setup to execute.
    /// @param dmacnt Current DMACNT value.
    /// @return Transfer type that this channel is set to.
    XferType DetermineStartTiming(DMACNT dmacnt) const;

    /// @brief Check if this channel is setup to perform FIFO transfers.
    /// @param dmacnt Current DMACNT value.
    /// @return True if this channel is setup to execute transfers to audio fifo A or B.
    bool IsFifoXfer(DMACNT dmacnt) const;

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Execution
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Execute a DMA transfer to/from EEPROM.
    /// @param dmacnt DMACNT register value.
    /// @param read True if this transfer is reading from EEPROM and writing to system memory.
    /// @param write True if this transfer is reading from system memory and writing to EEPROM.
    /// @return Number of cycles taken to complete the transfer.
    int ExecuteEepromXfer(DMACNT dmacnt, bool read, bool write);

    /// @brief Execute a DMA audio FIFO transfer.
    /// @param dmacnt DMACNT register value.
    /// @return Number of cycles taken to complete the transfer.
    int ExecuteFifoXfer(DMACNT dmacnt);

    /// @brief Execute a normal DMA transfer (any transfer except to EEPROM or an audio FIFO).
    /// @param dmacnt DMACNT register value.
    /// @return Number of cycles taken to perform the transfer.
    int ExecuteNormalXfer(DMACNT dmacnt);

    /// @brief Read a bit from memory as part of an EEPROM transfer and update internal registers.
    /// @return Next value from the bitstream (only LSB) and number of cycles taken to read.
    std::pair<u8, int> ReadForEepromXfer();

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Data
    ///-----------------------------------------------------------------------------------------------------------------------------

    // Memory access
    ReadMemCallback ReadMemory;
    WriteMemCallback WriteMemory;
    cartridge::GamePak* gamePakPtr_;

    // Channel info
    u8 const channelIndex_;
    InterruptType const interruptType_;

    // Registers
    std::array<std::byte, 12> registers_;
    u32 internalSrcAddr_;
    u32 internalDestAddr_;
    u32 internalWordCount_;
};
}  // namespace dma
