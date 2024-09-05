#pragma once

#include <array>
#include <bit>
#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/Types/DebugTypes.hpp>
#include <GBA/include/Types/Types.hpp>

namespace cpu
{
/// @brief Manager of ARM7TDMI registers.
class Registers
{
public:
    Registers(Registers const&) = delete;
    Registers& operator=(Registers const&) = delete;
    Registers(Registers&&) = delete;
    Registers& operator=(Registers&&) = delete;

    /// @brief Zero initialize all registers.
    Registers();

    /// @brief Read a general purpose register using the current operating mode according to CPSR.
    /// @param index Index of register to read.
    /// @return Current register value.
    u32 ReadRegister(u8 index) const;

    /// @brief Read a general purpose register corresponding to the register bank of the specified operating mode.
    /// @param index Index of register to read.
    /// @param mode Select register from the register bank corresponding to this operating mode.
    /// @return Current register value.
    u32 ReadRegister(u8 index, OperatingMode mode) const;

    /// @brief Set a general purpose register using the current operating mode according to CPSR.
    /// @param index Index of register to write to.
    /// @param val Value to write to register.
    void WriteRegister(u8 index, u32 val);

    /// @brief Set a general purpose register corresponding to the register bank of the specified operating mode.
    /// @param index Index of register to write to.
    /// @param val Value to write to register.
    /// @param mode Select register from the register bank corresponding to this operating mode.
    void WriteRegister(u8 index, u32 val, OperatingMode mode);

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// PC access
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Get the current value of the program counter.
    /// @return Current PC value.
    u32 GetPC() const { return sysAndUserRegBank_.r15; }

    /// @brief Set the value of the program counter. Should be accompanied by a pipeline flush.
    /// @param val Value to set PC to.
    void SetPC(u32 val) { WriteRegister(PC_INDEX, val); }

    /// @brief Increment the PC by either 2 or 4 depending on the current operating state.
    void AdvancePC() { sysAndUserRegBank_.r15 += InArmState() ? 4 : 2; }

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// CPSR flags access
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Check the current operating mode of the CPU.
    /// @return Current operating mode.
    OperatingMode GetOperatingMode() const { return OperatingMode{cpsr_.Mode}; }

    /// @brief Set the CPU's operating mode.
    /// @param mode New operating mode.
    void SetOperatingMode(OperatingMode mode) { cpsr_.Mode = static_cast<u32>(mode); }

    /// @brief Check the current operating state of the CPU.
    /// @return Current operating state.
    OperatingState GetOperatingState() const { return OperatingState{cpsr_.T}; }

    /// @brief Set the CPU's operating state.
    /// @param state New operating state.
    void SetOperatingState(OperatingState state) { cpsr_.T = static_cast<u32>(state); }

    /// @brief Check if the CPU is currently set to execute ARM instructions.
    /// @return True if CPU is in ARM state.
    bool InArmState() const { return GetOperatingState() == OperatingState::ARM; }

    /// @brief Check if the CPU is currently set to execute THUMB instructions.
    /// @return True if CPU is in THUMB state.
    bool InThumbState() const { return GetOperatingState() == OperatingState::THUMB; }

    /// @brief Check if Negative/Less Than flag is set.
    /// @return Current state of N flag.
    bool IsNegative() const { return cpsr_.N; }

    /// @brief Update the Negative/Less Than flag.
    /// @param state New value to set N flag to.
    void SetNegative(bool state) { cpsr_.N = state; }

    /// @brief Check if Zero flag is set.
    /// @return Current state of Z flag.
    bool IsZero() const { return cpsr_.Z; }

    /// @brief Update the Zero flag.
    /// @param state New value to set Z flag to.
    void SetZero(bool state) { cpsr_.Z = state; }

    /// @brief Check if Carry/Borrow/Extend flag is set.
    /// @return Current state of C flag.
    bool IsCarry() const { return cpsr_.C; }

    /// @brief Update the Carry/Borrow/Extend flag.
    /// @param state New value to set C flag to.
    void SetCarry(bool state) { cpsr_.C = state; }

    /// @brief Check if Overflow flag is set.
    /// @return Current state of V flag.
    bool IsOverflow() const { return cpsr_.V; }

    /// @brief Update the Overflow flag.
    /// @param state New value to set V flag to.
    void SetOverflow(bool state) { cpsr_.V = state; }

    /// @brief Check if IRQ interrupts are disabled.
    /// @return Whether IRQ interrupts are currently disabled.
    bool IsIrqDisabled() const { return cpsr_.I; }

    /// @brief Set the IRQ disabled flag.
    /// @param state New value to set I flag to. true = disabled, false = enabled.
    void SetIrqDisabled(bool state) { cpsr_.I = state; }

    /// @brief Check if FIQ interrupts are disabled.
    /// @return Whether FIQ interrupts are currently disabled.
    bool IsFiqDisabled() const { return cpsr_.F; }

    /// @brief Set the FIQ disabled flag.
    /// @param state New value to set F flag to. true = disabled, false = enabled.
    void SetFiqDisabled(bool state) { cpsr_.F = state; }

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// CPSR/SPSR getters/setters
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Get the raw value of the CPSR register.
    /// @return Current CPSR value.
    u32 GetCPSR() const { return std::bit_cast<u32>(cpsr_); }

    /// @brief Set the value of the CPSR register.
    /// @param val Raw value to set CPSR to.
    void SetCPSR(u32 val) { cpsr_ = std::bit_cast<CPSR>(val); }

    /// @brief Get the raw value of the SPSR register.
    /// @return Current SPSR value of the current operating mode.
    u32 GetSPSR() const;

    /// @brief Set teh value of the SPSR register of the current operating mode.
    /// @param val Raw value to set SPSR to.
    void SetSPSR(u32 val);

    /// @brief Copy the SPSR value of the current operating mode into CPSR.
    void LoadSPSR();

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Debug
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Get the current state of the ARM7TDMI registers.
    /// @param regState Reference to register state to populate.
    void GetRegState(debug::cpu::RegState& regState) const;

private:
    /// @brief Setup registers to start executing from ROM.
    void SkipBIOS();

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Register banks and lookup tables
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Bitfield of fields in CPSR and SPSR.
    struct CPSR
    {
        u32 Mode    : 5;
        u32 T       : 1;
        u32 F       : 1;
        u32 I       : 1;
        u32         : 20;
        u32 V       : 1;
        u32 C       : 1;
        u32 Z       : 1;
        u32 N       : 1;
    };

    static_assert(sizeof(CPSR) == sizeof(u32), "CPSR must be 4 bytes");

    CPSR cpsr_;

    struct
    {
        u32 r0;
        u32 r1;
        u32 r2;
        u32 r3;
        u32 r4;
        u32 r5;
        u32 r6;
        u32 r7;
        u32 r8;
        u32 r9;
        u32 r10;
        u32 r11;
        u32 r12;
        u32 r13;
        u32 r14;
        u32 r15;
    } sysAndUserRegBank_;

    struct
    {
        u32 r8;
        u32 r9;
        u32 r10;
        u32 r11;
        u32 r12;
        u32 r13;
        u32 r14;
        CPSR spsr;
    } fiqRegBank_;

    struct
    {
        u32 r13;
        u32 r14;
        CPSR spsr;
    } supervisorRegBank_;

    struct
    {
        u32 r13;
        u32 r14;
        CPSR spsr;
    } abortRegBank_;

    struct
    {
        u32 r13;
        u32 r14;
        CPSR spsr;
    } irqRegBank_;

    struct
    {
        u32 r13;
        u32 r14;
        CPSR spsr;
    } undefinedRegBank_;

    std::array<u32* const, 16> const sysAndUserRegLUT_ = {
        &sysAndUserRegBank_.r0,
        &sysAndUserRegBank_.r1,
        &sysAndUserRegBank_.r2,
        &sysAndUserRegBank_.r3,
        &sysAndUserRegBank_.r4,
        &sysAndUserRegBank_.r5,
        &sysAndUserRegBank_.r6,
        &sysAndUserRegBank_.r7,
        &sysAndUserRegBank_.r8,
        &sysAndUserRegBank_.r9,
        &sysAndUserRegBank_.r10,
        &sysAndUserRegBank_.r11,
        &sysAndUserRegBank_.r12,
        &sysAndUserRegBank_.r13,
        &sysAndUserRegBank_.r14,
        &sysAndUserRegBank_.r15
    };

    std::array<u32* const, 16> const fiqRegLUT_ = {
        &sysAndUserRegBank_.r0,
        &sysAndUserRegBank_.r1,
        &sysAndUserRegBank_.r2,
        &sysAndUserRegBank_.r3,
        &sysAndUserRegBank_.r4,
        &sysAndUserRegBank_.r5,
        &sysAndUserRegBank_.r6,
        &sysAndUserRegBank_.r7,
        &fiqRegBank_.r8,
        &fiqRegBank_.r9,
        &fiqRegBank_.r10,
        &fiqRegBank_.r11,
        &fiqRegBank_.r12,
        &fiqRegBank_.r13,
        &fiqRegBank_.r14,
        &sysAndUserRegBank_.r15
    };

    std::array<u32* const, 16> const supervisorRegLUT_ = {
        &sysAndUserRegBank_.r0,
        &sysAndUserRegBank_.r1,
        &sysAndUserRegBank_.r2,
        &sysAndUserRegBank_.r3,
        &sysAndUserRegBank_.r4,
        &sysAndUserRegBank_.r5,
        &sysAndUserRegBank_.r6,
        &sysAndUserRegBank_.r7,
        &sysAndUserRegBank_.r8,
        &sysAndUserRegBank_.r9,
        &sysAndUserRegBank_.r10,
        &sysAndUserRegBank_.r11,
        &sysAndUserRegBank_.r12,
        &supervisorRegBank_.r13,
        &supervisorRegBank_.r14,
        &sysAndUserRegBank_.r15
    };

    std::array<u32* const, 16> const abortRegLUT_ = {
        &sysAndUserRegBank_.r0,
        &sysAndUserRegBank_.r1,
        &sysAndUserRegBank_.r2,
        &sysAndUserRegBank_.r3,
        &sysAndUserRegBank_.r4,
        &sysAndUserRegBank_.r5,
        &sysAndUserRegBank_.r6,
        &sysAndUserRegBank_.r7,
        &sysAndUserRegBank_.r8,
        &sysAndUserRegBank_.r9,
        &sysAndUserRegBank_.r10,
        &sysAndUserRegBank_.r11,
        &sysAndUserRegBank_.r12,
        &abortRegBank_.r13,
        &abortRegBank_.r14,
        &sysAndUserRegBank_.r15
    };

    std::array<u32* const, 16> const irqRegLUT_ = {
        &sysAndUserRegBank_.r0,
        &sysAndUserRegBank_.r1,
        &sysAndUserRegBank_.r2,
        &sysAndUserRegBank_.r3,
        &sysAndUserRegBank_.r4,
        &sysAndUserRegBank_.r5,
        &sysAndUserRegBank_.r6,
        &sysAndUserRegBank_.r7,
        &sysAndUserRegBank_.r8,
        &sysAndUserRegBank_.r9,
        &sysAndUserRegBank_.r10,
        &sysAndUserRegBank_.r11,
        &sysAndUserRegBank_.r12,
        &irqRegBank_.r13,
        &irqRegBank_.r14,
        &sysAndUserRegBank_.r15
    };

    std::array<u32* const, 16> const undefinedRegLUT = {
        &sysAndUserRegBank_.r0,
        &sysAndUserRegBank_.r1,
        &sysAndUserRegBank_.r2,
        &sysAndUserRegBank_.r3,
        &sysAndUserRegBank_.r4,
        &sysAndUserRegBank_.r5,
        &sysAndUserRegBank_.r6,
        &sysAndUserRegBank_.r7,
        &sysAndUserRegBank_.r8,
        &sysAndUserRegBank_.r9,
        &sysAndUserRegBank_.r10,
        &sysAndUserRegBank_.r11,
        &sysAndUserRegBank_.r12,
        &undefinedRegBank_.r13,
        &undefinedRegBank_.r14,
        &sysAndUserRegBank_.r15
    };
};
}  // namespace cpu
