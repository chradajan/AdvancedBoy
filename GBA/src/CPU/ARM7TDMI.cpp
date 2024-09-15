#include <GBA/include/CPU/ARM7TDMI.hpp>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <GBA/include/CPU/ArmDisassembler.hpp>
#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/CPU/Registers.hpp>
#include <GBA/include/CPU/ThumbDisassembler.hpp>
#include <GBA/include/Logging/Logger.hpp>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/Types/DebugTypes.hpp>
#include <GBA/include/Types/Types.hpp>
#include <GBA/include/Utilities/CircularBuffer.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>
#include <GBA/include/Utilities/Functor.hpp>

namespace cpu
{
ARM7TDMI::ARM7TDMI(ReadMemCallback readMem,
                   WriteMemCallback writeMem,
                   EventScheduler& scheduler,
                   logging::Logger& log) :
    ReadMemory(readMem),
    WriteMemory(writeMem),
    flushPipeline_(false),
    scheduler_(scheduler),
    log_(log)
{
}

bool ARM7TDMI::Step(bool irq)
{
    if (irq && !registers_.IsIrqDisabled())
    {
        HandleIRQ();
    }

    bool armState = registers_.InArmState();
    AccessSize length = armState ? AccessSize::WORD : AccessSize::HALFWORD;

    // Fetch
    u32 fetchedPC = registers_.GetPC();
    auto [fetchedInstruction, cycles] = ReadMemory(fetchedPC, length);
    pipeline_.Push({fetchedInstruction, fetchedPC});
    scheduler_.Step(cycles);

    // Decode and execute
    if (pipeline_.Full())
    {
        auto [undecodedInstruction, executedPC] = pipeline_.Pop();

        if (log_.Enabled())
        {
            auto const& mnemonic = armState ? DisassembleArmInstruction(undecodedInstruction) :
                                              DisassembleThumbInstruction(undecodedInstruction);
            debug::cpu::RegState regState;
            registers_.GetRegState(regState);
            log_.LogCPU(mnemonic, regState, executedPC, undecodedInstruction, armState);
        }

        if (armState)
        {
            DecodeAndExecuteARM(undecodedInstruction);
        }
        else
        {
            DecodeAndExecuteTHUMB(undecodedInstruction);
        }
    }

    if (flushPipeline_)
    {
        pipeline_.Clear();
        flushPipeline_ = false;
    }
    else
    {
        registers_.AdvancePC();
    }

    return pipeline_.Size() >= 2;
}

u32 ARM7TDMI::GetNextAddrToExecute() const
{
    if (pipeline_.Empty())
    {
        return registers_.GetPC();
    }

    return pipeline_.PeakTail().PC;
}

debug::cpu::Mnemonic const& ARM7TDMI::DisassembleArmInstruction(u32 instruction)
{
    auto foundIter = decodedArmInstructions_.find(instruction);

    if (foundIter == decodedArmInstructions_.end())
    {
        bool inserted;
        auto mnemonic = arm::DisassembleInstruction(instruction);
        std::tie(foundIter, inserted) = decodedArmInstructions_.insert({instruction, mnemonic});
    }

    return foundIter->second;
}

debug::cpu::Mnemonic const& ARM7TDMI::DisassembleThumbInstruction(u16 instruction)
{
    auto foundIter = decodedThumbInstructions_.find(instruction);

    if (foundIter == decodedThumbInstructions_.end())
    {
        bool inserted;
        auto mnemonic = thumb::DisassembleInstruction(instruction);
        std::tie(foundIter, inserted) = decodedThumbInstructions_.insert({instruction, mnemonic});
    }

    return foundIter->second;
}

void ARM7TDMI::HandleIRQ()
{
    u32 cpsr = registers_.GetCPSR();
    u32 lr = pipeline_.Empty() ? registers_.GetPC() : pipeline_.PeakTail().PC;
    lr += 4;

    registers_.SetOperatingMode(OperatingMode::IRQ);
    registers_.SetOperatingState(OperatingState::ARM);
    registers_.WriteRegister(LR_INDEX, lr);
    registers_.SetIrqDisabled(true);
    registers_.SetSPSR(cpsr);
    registers_.SetPC(IRQ_VECTOR);
    pipeline_.Clear();
}

bool ARM7TDMI::ConditionSatisfied(u8 condition) const
{
    switch (condition)
    {
        case 0:  // EQ
            return registers_.IsZero();
        case 1:  // NE
            return !registers_.IsZero();
        case 2:  // CS
            return registers_.IsCarry();
        case 3:  // CC
            return !registers_.IsCarry();
        case 4:  // MI
            return registers_.IsNegative();
        case 5:  // PL
            return !registers_.IsNegative();
        case 6:  // VS
            return registers_.IsOverflow();
        case 7:  // VC
            return !registers_.IsOverflow();
        case 8:  // HI
            return registers_.IsCarry() && !registers_.IsZero();
        case 9:  // LS
            return !registers_.IsCarry() || registers_.IsZero();
        case 10: // GE
            return registers_.IsNegative() == registers_.IsOverflow();
        case 11: // LT
            return registers_.IsNegative() != registers_.IsOverflow();
        case 12: // GT
            return !registers_.IsZero() && (registers_.IsNegative() == registers_.IsOverflow());
        case 13: // LE
            return registers_.IsZero() || (registers_.IsNegative() != registers_.IsOverflow());
        case 14: // AL
            return true;
        default:
            throw std::runtime_error("Illegal ARM condition code");
    }

    return true;
}
}  // namespace cpu
