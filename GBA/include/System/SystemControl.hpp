#pragma once

#include <array>
#include <cstddef>
#include <cstring>
#include <GBA/include/Utilities/Types.hpp>

class EventScheduler;

/// @brief Possible waitstate regions to use for GamePak access.
enum class WaitStateRegion
{
    ZERO,
    ONE,
    TWO,
    SRAM
};

enum class InterruptType : u16
{
    LCD_VBLANK              = 0x0001,
    LCD_HBLANK              = 0x0002,
    LCD_VCOUNTER_MATCH      = 0x0004,
    TIMER_0_OVERFLOW        = 0x0008,
    TIMER_1_OVERFLOW        = 0x0010,
    TIMER_2_OVERFLOW        = 0x0020,
    TIMER_3_OVERFLOW        = 0x0040,
    SERIAL_COMMUNICATION    = 0x0080,
    DMA0                    = 0x0100,
    DMA1                    = 0x0200,
    DMA2                    = 0x0400,
    DMA3                    = 0x0800,
    KEYPAD                  = 0x1000,
    GAME_PAK                = 0x2000
};

/// @brief Manager of interrupt, waitstate, and power-down control registers.
class SystemControl
{
public:
    SystemControl() = delete;
    SystemControl(SystemControl const&) = delete;
    SystemControl& operator=(SystemControl const&) = delete;
    SystemControl(SystemControl&&) = delete;
    SystemControl& operator=(SystemControl&&) = delete;

    /// @brief Initialize the system control registers.
    /// @param scheduler Reference to event scheduler to post IRQ events to.
    explicit SystemControl(EventScheduler& scheduler);

    /// @brief Read an address mapped to system control registers.
    /// @param addr Address of system control register(s).
    /// @param length Memory access size of the read.
    /// @return Number of cycles taken to read, value of the requested register(s), and whether it was an open-bus read.
    MemReadData ReadReg(u32 addr, AccessSize length);

    /// @brief Write to an address mapped to system control registers.
    /// @param addr Address of system control register(s).
    /// @param val Value to write to register(s).
    /// @param length Memory access size of the write.
    /// @return Number of cycles taken to write.
    int WriteReg(u32 addr, u32 val, AccessSize length);

    /// @brief Set an interrupt flag in the IF register.
    /// @param interrupt Which interrupt type to request.
    void RequestInterrupt(InterruptType interrupt);

    /// @brief Check if the CPU is halted.
    /// @return Whether the CPU is currently halted.
    bool Halted() const { return halted_; }

    /// @brief Check if an IRQ is pending. True if IF & IE != 0, and IME is enabled.
    /// @return Whether an IRQ is pending.
    bool IrqPending() const { return irqPending_; }

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// WAITCNT access
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Get the number of wait states associated with a ROM or SRAM region.
    /// @param region Which wait state region to check.
    /// @param sequential Whether this access is sequential to the last one.
    /// @param length Memory access size.
    /// @return Number of additional wait states for the current read/write operation.
    int WaitStates(WaitStateRegion region, bool sequential, AccessSize length) const;

    /// @brief Check if the GamePak prefetcher is enabled.
    /// @return Whether prefetcher is currently enabled.
    bool GamePakPrefetchEnabled() const { return GetWAITCNT().prefetchBuffer; }

private:
    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Interrupts
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Get IE register.
    /// @return IE value.
    u16 GetIE() const { u16 reg; std::memcpy(&reg, &interruptAndWaitcntRegisters_[0], sizeof(u16)); return reg; }

    /// @brief Get IF register.
    /// @return IF value.
    u16 GetIF() const { u16 reg; std::memcpy(&reg, &interruptAndWaitcntRegisters_[2], sizeof(u16)); return reg; }

    /// @brief Set IF register.
    /// @param reg Value to set IF to.
    void SetIF(u16 reg) { std::memcpy(&interruptAndWaitcntRegisters_[2], &reg, sizeof(u16)); }

    /// @brief Check if interrupts are enabled.
    /// @return Whether interrupts are enabled.
    bool GetIME() const { return (static_cast<u8>(interruptAndWaitcntRegisters_[8]) & 0x01) == 0x01; }

    /// @brief Check if interrupts are enabled and an enabled interrupt type has been requested. Also unhalt if needed.
    void CheckForInterrupt();

    /// @brief Callback to set IRQ line to high.
    void SetIRQLine(int) { irqPending_ = true; }

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Special R/W functionality
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Read interrupt and WAITCNT registers taking unused ones into account.
    /// @param addr Address to read.
    /// @param length Memory access size of the read.
    /// @return Number of cycles taken to read, value of the requested register(s), and whether it was an open-bus read.
    MemReadData ReadInterruptAndWaitcntRegisters(u32 addr, AccessSize length);

    /// @brief Write to interrupt and WAITCNT registers. Handles IF acknowledgement, unused registers, and read-only bits.
    /// @param addr Address to write.
    /// @param val 
    /// @param length Memory access size of the write.
    void WriteInterruptAndWaitcntRegisters(u32 addr, u32 val, AccessSize length);

    /// @brief Read POSTFLG and HALTCNT registers taking unused and write-only ones into account.
    /// @param addr Address to read.
    /// @param length Memory access size of the read.
    /// @return Number of cycles taken to read, value of the requested register(s), and whether it was an open-bus read.
    MemReadData ReadPostFlgAndHaltcntRegisters(u32 addr, AccessSize length);

    /// @brief Write POSTFLG and HALTCNT registers. Halts the system if HALTCNT is written.
    /// @param addr Address to write.
    /// @param val Value to write.
    /// @param length Memory access size of the write.
    void WritePostFlgAndHaltcntRegisters(u32 addr, u32 val, AccessSize length);

    /// @brief Clear IF bits corresponding to acknowledged bits.
    /// @param ack Bits to clear in IF.
    void AcknowledgeInterrupt(u16 ack);

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Wait states
    ///-----------------------------------------------------------------------------------------------------------------------------

    struct WAITCNT
    {
        static constexpr size_t INDEX = 4;

        u16 sramWaitCtrl            : 2;
        u16 waitState0FirstAccess   : 2;
        u16 waitState0SecondAccess  : 1;
        u16 waitState1FirstAccess   : 2;
        u16 waitState1SecondAccess  : 1;
        u16 waitState2FirstAccess   : 2;
        u16 waitState2SecondAccess  : 1;
        u16 phiTerminalOutput       : 2;
        u16                         : 1;
        u16 prefetchBuffer          : 1;
        u16 gamePakType             : 1;
    };

    static_assert(sizeof(WAITCNT) == sizeof(u16), "WAITCNT must be 2 bytes");

    /// @brief Get the current value of the WAITCNT register.
    /// @return Current WAITCNT value.
    WAITCNT GetWAITCNT() const {
        WAITCNT reg; std::memcpy(&reg, &interruptAndWaitcntRegisters_[WAITCNT::INDEX], sizeof(WAITCNT)); return reg;
    }

    /// @brief Update teh value of the WAITCNT register.
    /// @param reg New value of WAITCNT.
    void SetWAITCNT(WAITCNT reg) { std::memcpy(&interruptAndWaitcntRegisters_[WAITCNT::INDEX], &reg, sizeof(WAITCNT)); }

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Data
    ///-----------------------------------------------------------------------------------------------------------------------------

    // State
    bool irqPending_;
    bool halted_;

    std::array<std::byte, 0x0C> interruptAndWaitcntRegisters_;
    std::array<std::byte, 0x04> postFlgAndHaltcntRegisters_;
    std::array<std::byte, 0x04> memoryControlRegisters_;

    // External components
    EventScheduler& scheduler_;
};
