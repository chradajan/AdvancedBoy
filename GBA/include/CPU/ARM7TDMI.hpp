#pragma once

#include <utility>
#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/CPU/Registers.hpp>
#include <GBA/include/Logging/Logger.hpp>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/Types.hpp>
#include <GBA/include/Utilities/CicrularBuffer.hpp>
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
    /// @param ReadMem Callback function to access bus read functionality.
    /// @param WriteMem Callback function to access bus write functionality.
    /// @param scheduler Reference to event scheduler that will be advanced as instructions execute.
    /// @param log Reference to logger to log CPU state and instructions to.
    explicit ARM7TDMI(ReadMemCallback readMem, WriteMemCallback writeMem, EventScheduler& scheduler, logging::Logger& log);

    /// @brief Advance the pipeline by one stage and execute an instruction if one is ready to be executed. Advances the scheduler
    ///        after each memory read/write and internal cycle.
    /// @param irq Status of IRQ line. True if an IRQ is pending.
    void Step(bool irq);

    /// @brief Get the current PC value that code is executing from.
    /// @return Current r15 value.
    u32 GetPC() { return registers_.GetPC(); }

    /// @brief Set whether the CPU should log its state and decoded instructions.
    /// @param enable Whether CPU should log.
    void SetLogging(bool enable) { loggingEnabled_ = enable; }

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

    /// @brief Decode an ARM instruction, optionally log it along with the current state of registers, and execute the instruction.
    /// @param instruction Undecoded 32-bit ARM instruction.
    /// @param log Whether to log the instruction.
    void DecodeAndExecuteARM(u32 instruction, bool log);

    void ExecuteBranchAndExchange(u32 instruction);                     void LogBranchAndExchange(u32 instruction) const;
    void ExecuteBlockDataTransfer(u32 instruction);                     void LogBlockDataTransfer(u32 instruction) const;
    void ExecuteBranch(u32 instruction);                                void LogBranch(u32 instruction) const;
    void ExecuteArmSoftwareInterrupt(u32 instruction);                  void LogArmSoftwareInterrupt(u32 instruction) const;
    void ExecuteUndefined(u32 instruction);                             void LogUndefined(u32 instruction) const;
    void ExecuteSingleDataTransfer(u32 instruction);                    void LogSingleDataTransfer(u32 instruction) const;
    void ExecuteSingleDataSwap(u32 instruction);                        void LogSingleDataSwap(u32 instruction) const;
    void ExecuteMultiply(u32 instruction);                              void LogMultiply(u32 instruction) const;
    void ExecuteMultiplyLong(u32 instruction);                          void LogMultiplyLong(u32 instruction) const;
    void ExecuteHalfwordDataTransfer(u32 instruction);                  void LogHalfwordDataTransfer(u32 instruction) const;
    void ExecutePSRTransferMRS(u32 instruction);                        void LogPSRTransferMRS(u32 instruction) const;
    void ExecutePSRTransferMSR(u32 instruction);                        void LogPSRTransferMSR(u32 instruction) const;
    void ExecuteDataProcessing(u32 instruction);                        void LogDataProcessing(u32 instruction) const;

    ///-----------------------------------------------------------------------------------------------------------------------------
    /// THUMB instructions
    ///-----------------------------------------------------------------------------------------------------------------------------

    /// @brief Decode a THUMB instruction, optionally log it along with the current state of registers, and execute the instruction.
    /// @param instruction Undecoded 16-bit THUMB instruction.
    /// @param log Whether to log the instruction.
    void DecodeAndExecuteTHUMB(u16 instruction, bool log);

    void ExecuteThumbSoftwareInterrupt(u16 instruction);                void LogThumbSoftwareInterrupt(u16 instruction) const;
    void ExecuteUnconditionalBranch(u16 instruction);                   void LogUnconditionalBranch(u16 instruction) const;
    void ExecuteConditionalBranch(u16 instruction);                     void LogConditionalBranch(u16 instruction) const;
    void ExecuteMultipleLoadStore(u16 instruction);                     void LogMultipleLoadStore(u16 instruction) const;
    void ExecuteLongBranchWithLink(u16 instruction);                    void LogLongBranchWithLink(u16 instruction) const;
    void ExecuteAddOffsetToStackPointer(u16 instruction);               void LogAddOffsetToStackPointer(u16 instruction) const;
    void ExecutePushPopRegisters(u16 instruction);                      void LogPushPopRegisters(u16 instruction) const;
    void ExecuteLoadStoreHalfword(u16 instruction);                     void LogLoadStoreHalfword(u16 instruction) const;
    void ExecuteSPRelativeLoadStore(u16 instruction);                   void LogSPRelativeLoadStore(u16 instruction) const;
    void ExecuteLoadAddress(u16 instruction);                           void LogLoadAddress(u16 instruction) const;
    void ExecuteLoadStoreWithOffset(u16 instruction);                   void LogLoadStoreWithOffset(u16 instruction) const;
    void ExecuteLoadStoreSignExtendedByteHalfword(u16 instruction);     void LogLoadStoreSignExtendedByteHalfword(u16 instruction) const;
    void ExecutePCRelativeLoad(u16 instruction);                        void LogPCRelativeLoad(u16 instruction) const;
    void ExecuteHiRegisterOperationsBranchExchange(u16 instruction);    void LogHiRegisterOperationsBranchExchange(u16 instruction) const;
    void ExecuteALUOperations(u16 instruction);                         void LogALUOperations(u16 instruction) const;
    void ExecuteMoveCompareAddSubtractImmediate(u16 instruction);       void LogMoveCompareAddSubtractImmediate(u16 instruction) const;
    void ExecuteAddSubtract(u16 instruction);                           void LogAddSubtract(u16 instruction) const;
    void ExecuteMoveShiftedRegister(u16 instruction);                   void LogMoveShiftedRegister(u16 instruction) const;

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

    // Logging
    logging::Logger& log_;
    bool loggingEnabled_;
    u32 logPC_;
};
}  // namespace cpu
