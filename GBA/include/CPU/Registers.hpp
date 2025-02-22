#pragma once

#include <array>
#include <bit>
#include <fstream>
#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/Debug/DebugTypes.hpp>
#include <GBA/include/Utilities/Types.hpp>

namespace debug { class CPUDebugger; }

namespace cpu
{
/// @brief Manager of ARM7TDMI registers.
class Registers
{
public:
    Registers() = delete;
    Registers(Registers const&) = delete;
    Registers& operator=(Registers const&) = delete;
    Registers(Registers&&) = delete;
    Registers& operator=(Registers&&) = delete;

    /// @brief Initialize ARM7TDMI registers.
    /// @param skipBiosIntro Whether to initialize registers to their post-BIOS state.
    explicit Registers(bool skipBiosIntro);

    /// @brief Read a general purpose register using the current operating mode according to CPSR.
    /// @param index Index of register to read.
    /// @return Current register value.
    u32 ReadRegister(u8 index) const { return gpRegisters_[index]; }

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
    u32 GetPC() const { return gpRegisters_[PC_INDEX]; }

    /// @brief Set the value of the program counter. Should be accompanied by a pipeline flush.
    /// @param val Value to set PC to.
    void SetPC(u32 val) { WriteRegister(PC_INDEX, val); }

    /// @brief Increment the PC by either 2 or 4 depending on the current operating state.
    void AdvancePC() { gpRegisters_[PC_INDEX] += InArmState() ? 4 : 2; }

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// CPSR flags access
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Check the current operating mode of the CPU.
    /// @return Current operating mode.
    OperatingMode GetOperatingMode() const { return OperatingMode{cpsr_.Mode}; }

    /// @brief Set the CPU's operating mode.
    /// @param mode New operating mode.
    void SetOperatingMode(OperatingMode mode);

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
    void SetCPSR(u32 val);

    /// @brief Get the raw value of the SPSR register.
    /// @return Current SPSR value of the current operating mode.
    u32 GetSPSR() const { return std::bit_cast<u32>(spsr_); }

    /// @brief Set the value of the SPSR register of the current operating mode.
    /// @param val Raw value to set SPSR to.
    void SetSPSR(u32 val) { spsr_ = std::bit_cast<CPSR>(val); }

    /// @brief Copy the SPSR value of the current operating mode into CPSR.
    void LoadSPSR() { SetCPSR(std::bit_cast<u32>(spsr_)); }

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Save States
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Write data to save state file.
    /// @param saveState Save state stream to write to.
    void Serialize(std::ofstream& saveState) const;

    /// @brief Load data from save state file.
    /// @param saveState Save state stream to read from.
    void Deserialize(std::ifstream& saveState);

private:
    /// @brief Setup registers to start executing from ROM.
    void SkipBIOS();

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Program status registers
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
    CPSR spsr_;

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// General purpose registers
    ///-----------------------------------------------------------------------------------------------------------------------------

    std::array<u32, 16> gpRegisters_;
    std::array<u32, 8> fiqRegisters_;  // R8_fiq - R14_fiq, SPSR_fiq
    std::array<u32, 3> svcRegisters_;  // R13_svc - R14_svc, SPSR_svc
    std::array<u32, 3> abtRegisters_;  // R13_abt - R14_abt, SPSR_abt
    std::array<u32, 3> irqRegisters_;  // R13_irq - R14_irq, SPSR_irq
    std::array<u32, 3> undRegisters_;  // R13_und - R14_und, SPSR_und

    // Debug
    friend class debug::CPUDebugger;
};
}  // namespace cpu
