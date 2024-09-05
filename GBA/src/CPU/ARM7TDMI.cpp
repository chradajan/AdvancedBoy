#include <GBA/include/CPU/ARM7TDMI.hpp>
#include <memory>
#include <stdexcept>
#include <GBA/include/CPU/ArmDisassembler.hpp>
#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/CPU/Registers.hpp>
#include <GBA/include/CPU/ThumbDisassembler.hpp>
#include <GBA/include/Logging/Logger.hpp>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/Types/Types.hpp>
#include <GBA/include/Utilities/CircularBuffer.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>
#include <GBA/include/Utilities/Functor.hpp>

namespace cpu
{
ARM7TDMI::ARM7TDMI(ReadMemCallback readMem,
                   WriteMemCallback writeMem,
                   FastMemAccessCallback fastMem,
                   EventScheduler& scheduler,
                   logging::Logger& log) :
    ReadMemory(readMem),
    WriteMemory(writeMem),
    flushPipeline_(false),
    scheduler_(scheduler),
    GetFastMem(fastMem),
    log_(log),
    updateDebugInfo_(false)
{
    if (log_.Enabled() || updateDebugInfo_)
    {
        RefillDebugInfo();
    }
}

void ARM7TDMI::Step(bool irq)
{
    bool debuggingEnabled = log_.Enabled() || updateDebugInfo_;

    if (irq && !registers_.IsIrqDisabled())
    {
        HandleIRQ();

        if (debuggingEnabled)
        {
            RefillDebugInfo();
        }
    }

    AccessSize length = registers_.InArmState() ? AccessSize::WORD : AccessSize::HALFWORD;

    // Fetch
    u32 fetchedPC = registers_.GetPC();
    auto [fetchedInstruction, cycles] = ReadMemory(fetchedPC, length);
    pipeline_.Push({fetchedInstruction, fetchedPC});
    scheduler_.Step(cycles);

    // Decode and execute
    if (pipeline_.Full())
    {
        auto [undecodedInstruction, executedPC] = pipeline_.Pop();

        if (log_.Enabled() && debugInfo_.currInstruction.has_value())
        {
            log_.LogCPU(debugInfo_.currInstruction.value(), debugInfo_.regState);
        }

        if (debuggingEnabled)
        {
            UpdateDebugInfo();
        }

        if (registers_.InArmState())
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

        if (debuggingEnabled)
        {
            RefillDebugInfo();
        }
    }
    else
    {
        registers_.AdvancePC();
    }
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

void ARM7TDMI::UpdateDebugInfo()
{
    if (debugInfo_.prevInstructions.Full())
    {
        debugInfo_.prevInstructions.Pop();
    }

    debugInfo_.prevInstructions.Push(debugInfo_.currInstruction.value());
    debugInfo_.currInstruction = debugInfo_.nextInstructions.Pop();
    registers_.GetRegState(debugInfo_.regState);

    u32 addr = debugInfo_.nextInstructions.PeakHead().addr;
    bool armState = registers_.InArmState();
    u8 delta = armState ? 4 : 2;
    addr += delta;
    u32 index = fastMemAccess_.AddrToIndex(addr);

    if ((index + delta) <= fastMemAccess_.memoryBlock.size())
    {
        u32 instruction = armState ? MemCpyInit<u32>(&fastMemAccess_.memoryBlock[index]) :
                                     MemCpyInit<u16>(&fastMemAccess_.memoryBlock[index]);

        debugInfo_.nextInstructions.Push(armState ? arm::DisassembleInstruction(instruction, addr) :
                                                    thumb::DisassembleInstruction(instruction, addr));
    }
}

void ARM7TDMI::RefillDebugInfo()
{
    u32 addr = registers_.GetPC();
    bool armState = registers_.InArmState();
    u8 delta = armState ? 4 : 2;
    fastMemAccess_ = GetFastMem(addr);
    u32 index = fastMemAccess_.AddrToIndex(addr);

    debugInfo_.prevInstructions.Clear();
    debugInfo_.currInstruction = {};
    debugInfo_.nextInstructions = {};
    registers_.GetRegState(debugInfo_.regState);

    if ((index + delta) <= fastMemAccess_.memoryBlock.size())
    {
        u32 instruction = armState ? MemCpyInit<u32>(&fastMemAccess_.memoryBlock[index]) :
                                     MemCpyInit<u16>(&fastMemAccess_.memoryBlock[index]);

        debugInfo_.currInstruction = armState ? arm::DisassembleInstruction(instruction, addr) :
                                                thumb::DisassembleInstruction(instruction, addr);
    }
    else
    {
        debugInfo_.currInstruction.emplace(debug::cpu::DisassembledInstruction{armState, 0, addr, "???", "???", "???"});
    }

    while (!debugInfo_.nextInstructions.Full())
    {
        addr += delta;
        index = fastMemAccess_.AddrToIndex(addr);

        if ((index + delta) <= fastMemAccess_.memoryBlock.size())
        {
            u32 instruction = armState ? MemCpyInit<u32>(&fastMemAccess_.memoryBlock[index]) :
                                         MemCpyInit<u16>(&fastMemAccess_.memoryBlock[index]);

            debugInfo_.nextInstructions.Push(armState ? arm::DisassembleInstruction(instruction, addr) :
                                                        thumb::DisassembleInstruction(instruction, addr));
        }
        else
        {
            debugInfo_.nextInstructions.Push(debug::cpu::DisassembledInstruction{armState, 0, addr, "???", "???", "???"});
        }
    }
}
}  // namespace cpu
