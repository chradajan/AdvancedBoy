#pragma once

#include <functional>
#include <unordered_map>
#include <utility>
#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/CPU/Registers.hpp>
#include <GBA/include/Logging/Logger.hpp>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/Types/DebugTypes.hpp>
#include <GBA/include/Types/Types.hpp>
#include <GBA/include/Utilities/CircularBuffer.hpp>
#include <GBA/include/Utilities/Functor.hpp>

class GameBoyAdvance;

namespace cpu
{
/// @brief ARM7TDMI CPU.
class ARM7TDMI
{
    using ReadMemCallback = MemberFunctor<std::pair<u32, int> (GameBoyAdvance::*)(u32, AccessSize)>;
    using WriteMemCallback = MemberFunctor<int (GameBoyAdvance::*)(u32, u32, AccessSize)>;

public:
    ARM7TDMI() = delete;
    ARM7TDMI(ARM7TDMI const&) = delete;
    ARM7TDMI& operator=(ARM7TDMI const&) = delete;
    ARM7TDMI(ARM7TDMI&&) = delete;
    ARM7TDMI& operator=(ARM7TDMI&&) = delete;

    /// @brief Initialize an ARM7TDMI CPU.
    /// @param readMem Callback function to access bus read functionality.
    /// @param writeMem Callback function to access bus write functionality.
    /// @param scheduler Reference to event scheduler that will be advanced as instructions execute.
    /// @param log Reference to logger to log CPU state and instructions to.
    explicit ARM7TDMI(ReadMemCallback readMem,
                      WriteMemCallback writeMem,
                      EventScheduler& scheduler,
                      logging::Logger& log);

    /// @brief Advance the pipeline by one stage and execute an instruction if one is ready to be executed. Advances the scheduler
    ///        after each memory read/write and internal cycle.
    /// @param irq Status of IRQ line. True if an IRQ is pending.
    /// @return Whether the next call to Step will result in an instruction being executed.
    bool Step(bool irq);

    /// @brief Get the current PC value that code is executing from.
    /// @return Current value of R15.
    u32 GetPC() { return registers_.GetPC(); }

    /// @brief Get the current SP value (R13).
    /// @return Current value of R13.
    u32 GetSP() { return registers_.ReadRegister(SP_INDEX); }

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Debug
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Get the current state of the ARM7TDMI registers.
    /// @param regState Reference to register state to populate.
    void GetRegState(debug::cpu::RegState& regState) const { registers_.GetRegState(regState); }

    /// @brief Get the address of the next instruction that will be executed.
    /// @return Address of next instruction to execute.
    u32 GetNextAddrToExecute() const;

    /// @brief Disassemble an ARM instruction into its human-readable mnemonic.
    /// @param instruction Raw 32-bit ARM instruction code.
    /// @return Disassembled instruction.
    debug::cpu::Mnemonic const& DisassembleArmInstruction(u32 instruction);

    /// @brief Disassemble a THUMB instruction into its human-readable mnemonic.
    /// @param instruction Raw 16-bit THUMB instruction code.
    /// @return Disassembled instruction.
    debug::cpu::Mnemonic const& DisassembleThumbInstruction(u16 instruction);

private:
    /// @brief Flush pipeline and prepare to start executing from IRQ handler.
    void HandleIRQ();

    /// @brief Check if an ARM instruction or THUMB conditional branch should be executed.
    /// @param condition Instruction code to evaluate.
    /// @return True if instruction should be executed.
    bool ConditionSatisfied(u8 condition) const;

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// ARM instructions
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Decode an ARM instruction and execute the instruction.
    /// @param instruction Undecoded 32-bit ARM instruction.
    void DecodeAndExecuteARM(u32 instruction);

    void ExecuteBranchAndExchange(u32 instruction);
    void ExecuteBlockDataTransfer(u32 instruction);
    void ExecuteBranch(u32 instruction);
    void ExecuteArmSoftwareInterrupt(u32 instruction);
    void ExecuteUndefined(u32 instruction);
    void ExecuteSingleDataTransfer(u32 instruction);
    void ExecuteSingleDataSwap(u32 instruction);
    void ExecuteMultiply(u32 instruction);
    void ExecuteMultiplyLong(u32 instruction);
    void ExecuteHalfwordDataTransfer(u32 instruction);
    void ExecutePSRTransferMRS(u32 instruction);
    void ExecutePSRTransferMSR(u32 instruction);
    void ExecuteDataProcessing(u32 instruction);

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// THUMB instructions
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Decode a THUMB instruction and execute the instruction.
    /// @param instruction Undecoded 16-bit THUMB instruction.
    void DecodeAndExecuteTHUMB(u16 instruction);

    void ExecuteThumbSoftwareInterrupt(u16 instruction);
    void ExecuteUnconditionalBranch(u16 instruction);
    void ExecuteConditionalBranch(u16 instruction);
    void ExecuteMultipleLoadStore(u16 instruction);
    void ExecuteLongBranchWithLink(u16 instruction);
    void ExecuteAddOffsetToStackPointer(u16 instruction);
    void ExecutePushPopRegisters(u16 instruction);
    void ExecuteLoadStoreHalfword(u16 instruction);
    void ExecuteSPRelativeLoadStore(u16 instruction);
    void ExecuteLoadAddress(u16 instruction);
    void ExecuteLoadStoreWithOffset(u16 instruction);
    void ExecuteLoadStoreSignExtendedByteHalfword(u16 instruction);
    void ExecutePCRelativeLoad(u16 instruction);
    void ExecuteHiRegisterOperationsBranchExchange(u16 instruction);
    void ExecuteALUOperations(u16 instruction);
    void ExecuteMoveCompareAddSubtractImmediate(u16 instruction);
    void ExecuteAddSubtract(u16 instruction);
    void ExecuteMoveShiftedRegister(u16 instruction);

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Bus access
    ///-----------------------------------------------------------------------------------------------------------------------------

    ReadMemCallback ReadMemory;
    WriteMemCallback WriteMemory;

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// CPU state
    ///-----------------------------------------------------------------------------------------------------------------------------

    // ARM registers
    Registers registers_;

    // Pipeline state
    CircularBuffer<PrefetchedInstruction, 3> pipeline_;
    bool flushPipeline_;

    // External components
    EventScheduler& scheduler_;

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// Debug
    ///-----------------------------------------------------------------------------------------------------------------------------

    logging::Logger& log_;
    std::unordered_map<u32, debug::cpu::Mnemonic> decodedArmInstructions_;
    std::unordered_map<u16, debug::cpu::Mnemonic> decodedThumbInstructions_;
};
}  // namespace cpu
