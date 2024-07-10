#pragma once

#include <utility>
#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/CPU/Registers.hpp>
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
    explicit ARM7TDMI(ReadMemCallback readMem, WriteMemCallback writeMem, EventScheduler& scheduler);

    /// @brief Advance the pipeline by one stage and execute an instruction if one is ready to be executed. Advances the scheduler
    ///        after each memory read/write and internal cycle.
    /// @param irq Status of IRQ line. True if an IRQ is pending.
    void Step(bool irq);

    /// @brief Get the current PC value that code is executing from.
    /// @return Current r15 value.
    u32 GetPC() { return registers_.GetPC(); }

private:
    /// @brief Flush pipeline and prepare to start executing from IRQ handler.
    void HandleIRQ();

    /// @brief Check if an ARM instruction or THUMB conditional branch should be executed.
    /// @param condition Instruction code to evaluate.
    /// @return True if instruction should be executed.
    bool ConditionSatisfied(u8 condition) const;

    ReadMemCallback ReadMemory;
    WriteMemCallback WriteMemory;

    // ARM registers
    Registers registers_;

    // Pipeline state
    CircularBuffer<PrefetchedInstruction, 3> pipeline_;
    bool flushPipeline_;

    // External components
    EventScheduler& scheduler_;
};
}  // namespace cpu
