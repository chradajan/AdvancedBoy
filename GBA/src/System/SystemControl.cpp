#include <GBA/include/System/SystemControl.hpp>
#include <array>
#include <bit>
#include <cstddef>
#include <cstring>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/Logging/Logger.hpp>
#include <GBA/include/Types.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>

static constexpr int NonSequentialWaitStates[4] = {4, 3, 2, 8};
static constexpr int SequentialWaitStates[3][2] = { {2, 1}, {4, 1}, {8, 1} };

SystemControl::SystemControl(logging::Logger& log) : log_(log)
{
    irqPending_ = false;
    halted_ = false;

    interruptAndWaitcntRegisters_.fill(std::byte{0});
    postFlgAndHaltcntRegisters_.fill(std::byte{0});
    memoryControlRegisters_.fill(std::byte{0});
}

MemReadData SystemControl::ReadReg(u32 addr, AccessSize length)
{
    MemReadData readData = {1, 0, false};

    switch (addr)
    {
        case INT_WAITCNT_ADDR_MIN ... INT_WAITCNT_ADDR_MAX:
            readData = ReadInterruptAndWaitcntRegisters(addr, length);
            break;
        case POSTFLG_HALTCNT_ADDR_MIN ... POSTFLG_HALTCNT_ADDR_MAX:
            readData = ReadPostFlgAndHaltcntRegisters(addr, length);
            break;
        case INTERNAL_MEM_CONTROL_ADDR_MIN ... INTERNAL_MEM_CONTROL_ADDR_MAX:
            readData.Value = ReadMemoryBlock(memoryControlRegisters_, addr, INTERNAL_MEM_CONTROL_ADDR_MIN, length);
            break;
        default:
            readData.OpenBus = true;
            break;
    }

    return readData;
}

int SystemControl::WriteReg(u32 addr, u32 val, AccessSize length)
{
    switch (addr)
    {
        case INT_WAITCNT_ADDR_MIN ... INT_WAITCNT_ADDR_MAX:
            WriteInterruptAndWaitcntRegisters(addr, val, length);
            break;
        case POSTFLG_HALTCNT_ADDR_MIN ... POSTFLG_HALTCNT_ADDR_MAX:
            WritePostFlgAndHaltcntRegisters(addr, val, length);
            break;
        case INTERNAL_MEM_CONTROL_ADDR_MIN ... INTERNAL_MEM_CONTROL_ADDR_MAX:
            WriteMemoryBlock(memoryControlRegisters_, addr, INTERNAL_MEM_CONTROL_ADDR_MIN, val, length);
            break;
        default:
            break;
    }

    return 1;
}

void SystemControl::RequestInterrupt(InterruptType interrupt)
{
    u16 IF = GetIF();

    if (log_.Enabled())
    {
        log_.LogInterruptRequest(static_cast<u16>(interrupt), GetIE(), GetIME());
    }

    IF |= static_cast<u16>(interrupt);
    SetIF(IF);
    CheckForInterrupt();
}

int SystemControl::WaitStates(WaitStateRegion region, bool sequential, AccessSize length) const
{
    auto waitcnt = GetWAITCNT();
    int firstAccess = 0;
    int secondAccess = 0;

    switch (region)
    {
        case WaitStateRegion::ZERO:
        {
            firstAccess = sequential ? SequentialWaitStates[0][waitcnt.waitState0SecondAccess] :
                                       NonSequentialWaitStates[waitcnt.waitState0FirstAccess];

            if (length == AccessSize::WORD)
            {
                secondAccess = SequentialWaitStates[0][waitcnt.waitState0SecondAccess];
            }

            break;
        }
        case WaitStateRegion::ONE:
        {
            firstAccess = sequential ? SequentialWaitStates[1][waitcnt.waitState1SecondAccess] :
                                       NonSequentialWaitStates[waitcnt.waitState1FirstAccess];

            if (length == AccessSize::WORD)
            {
                secondAccess = SequentialWaitStates[1][waitcnt.waitState1SecondAccess];
            }

            break;
        }
        case WaitStateRegion::TWO:
        {
            firstAccess = sequential ? SequentialWaitStates[2][waitcnt.waitState2SecondAccess] :
                                       NonSequentialWaitStates[waitcnt.waitState2FirstAccess];

            if (length == AccessSize::WORD)
            {
                secondAccess = SequentialWaitStates[2][waitcnt.waitState2SecondAccess];
            }

            break;
        }
        case WaitStateRegion::SRAM:
            firstAccess = NonSequentialWaitStates[waitcnt.sramWaitCtrl];
            break;
    }

    return firstAccess + secondAccess;
}

void SystemControl::CheckForInterrupt()
{
    irqPending_ = false;

    if ((GetIE() & GetIF()) != 0)
    {
        // TODO: Instead of instantly setting IRQ line, delay by however many cycles this should actually take.
        irqPending_ = GetIME();

        if (halted_)
        {
            if (log_.Enabled())
            {
                log_.LogHalt(false, GetIE(), GetIF());
            }

            halted_ = false;
        }
    }
}

MemReadData SystemControl::ReadInterruptAndWaitcntRegisters(u32 addr, AccessSize length)
{
    if (((0x0400'0206 <= addr) && (addr < 0x0400'0208)) || ((0x0400'020A <= addr) && (addr < 0x0400'020C)))
    {
        return {1, 0, false};
    }

    u32 val = ReadMemoryBlock(interruptAndWaitcntRegisters_, addr, INT_WAITCNT_ADDR_MIN, length);
    return {1, val, false};
}

void SystemControl::WriteInterruptAndWaitcntRegisters(u32 addr, u32 val, AccessSize length)
{
    if (addr < 0x0400'0204)  // Writing IE, possible writing IF
    {
        if (addr < 0x0400'0202)  // // Writing IE, possible writing IF
        {
            if (length == AccessSize::WORD)
            {
                WriteMemoryBlock(interruptAndWaitcntRegisters_, addr, INT_WAITCNT_ADDR_MIN, val, AccessSize::HALFWORD);
                AcknowledgeInterrupt(val >> 16);
            }
            else
            {
                WriteMemoryBlock(interruptAndWaitcntRegisters_, addr, INT_WAITCNT_ADDR_MIN, val, length);
            }
        }
        else  // Only writing IF
        {
            if (length == AccessSize::BYTE)
            {
                val = (addr == 0x0400'0202) ? (val & U8_MAX) : ((val & U8_MAX) << 8);
            }

            AcknowledgeInterrupt(val);
        }
    }
    else
    {
        WriteMemoryBlock(interruptAndWaitcntRegisters_, addr, INT_WAITCNT_ADDR_MIN, val, length);

        // Zero out unused registers
        std::memset(&interruptAndWaitcntRegisters_[6], 0, 2);
        std::memset(&interruptAndWaitcntRegisters_[10], 0, 2);

        // Force GamePak type flag to GBA
        auto waitcnt = GetWAITCNT();
        waitcnt.gamePakType = 0;
        SetWAITCNT(waitcnt);
    }

    CheckForInterrupt();
}

MemReadData SystemControl::ReadPostFlgAndHaltcntRegisters(u32 addr, AccessSize length)
{
    if ((addr > POSTFLG_HALTCNT_ADDR_MIN) || (length != AccessSize::BYTE))
    {
        return {1, 0, false};
    }

    return {1, static_cast<u32>(postFlgAndHaltcntRegisters_[0]), false};
}

void SystemControl::WritePostFlgAndHaltcntRegisters(u32 addr, u32 val, AccessSize length)
{
    bool checkHalt = false;

    if (addr < 0x0400'0302)
    {
        if (length == AccessSize::WORD)
        {
            length = AccessSize::HALFWORD;
            checkHalt = true;
        }
        else if ((length == AccessSize::HALFWORD) || (addr == 0x0400'0301))
        {
            checkHalt = true;
        }

        WriteMemoryBlock(postFlgAndHaltcntRegisters_, addr, POSTFLG_HALTCNT_ADDR_MIN, val, length);
    }

    if (checkHalt)
    {
        halted_ = (static_cast<u8>(postFlgAndHaltcntRegisters_[1]) & U8_MSB) == 0;

        if (halted_ && log_.Enabled())
        {
            log_.LogHalt(true, GetIE(), GetIF());
        }
    }
}

void SystemControl::AcknowledgeInterrupt(u16 ack)
{
    u16 IF = GetIF() & ~ack;
    SetIF(IF);
}
