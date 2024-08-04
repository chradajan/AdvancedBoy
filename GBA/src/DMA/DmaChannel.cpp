#include <GBA/include/DMA/DmaChannel.hpp>
#include <GBA/include/Cartridge/GamePak.hpp>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Types.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>

namespace dma
{
DmaChannel::DmaChannel(u8 index, InterruptType interrupt, ReadMemCallback readMem, WriteMemCallback writeMem) :
    ReadMemory(readMem),
    WriteMemory(writeMem),
    gamePakPtr_(nullptr),
    channelIndex_(index),
    interruptType_(interrupt)
{
    registers_.fill(std::byte{0});
    internalSrcAddr_ = 0;
    internalDestAddr_ = 0;
    internalWordCount_ = 0;
}

MemReadData DmaChannel::ReadReg(u32 addr, AccessSize length)
{
    u8 index = (addr - DMA_IO_ADDR_MIN) % 12;
    u32 val;
    bool openBus;

    if ((index == 8) && (length == AccessSize::WORD))
    {
        val = ReadMemoryBlock(registers_, 10, 0, AccessSize::HALFWORD) << 16;
        openBus = false;
    }
    else if (index < 10)
    {
        val = 0;
        openBus = true;
    }
    else
    {
        val = ReadMemoryBlock(registers_, index, 0, length);
        openBus = false;
    }

    return {1, val, openBus};
}

XferType DmaChannel::WriteReg(u32 addr, u32 val, AccessSize length)
{
    u8 index = (addr - DMA_IO_ADDR_MIN) % 12;
    auto prevDmaCnt = GetDMACNT();
    WriteMemoryBlock(registers_, index, 0, val, length);
    auto currDmaCnt = GetDMACNT();
    auto dmaState = XferType::NO_CHANGE;

    if (!prevDmaCnt.enable && currDmaCnt.enable)
    {
        internalSrcAddr_ = GetSAD() & ((channelIndex_ == 0) ? 0x07FF'FFFF : 0x0FFF'FFFF);
        internalDestAddr_ = GetDAD() & ((channelIndex_ == 3) ? 0x0FFF'FFFF : 0x07FF'FFFF);
        internalWordCount_ = GetWordCount() & ((channelIndex_ == 3) ? 0xFFFF : 0x3FFF);

        if (internalWordCount_ == 0)
        {
            internalWordCount_ = (channelIndex_ == 3) ? 0x0001'0000 : 0x4000;
        }

        dmaState = DetermineStartTiming(currDmaCnt);
    }
    else if (prevDmaCnt.enable && !currDmaCnt.enable)
    {
        dmaState = XferType::DISABLED;
    }
    else if (prevDmaCnt.enable && currDmaCnt.enable && (prevDmaCnt.timing != currDmaCnt.timing))
    {
        dmaState = DetermineStartTiming(currDmaCnt);
    }

    return dmaState;
}

ExecuteResult DmaChannel::Execute()
{
    auto dmacnt = GetDMACNT();
    bool eepromRead = false;
    bool eepromWrite = false;
    bool fifoXfer = IsFifoXfer(dmacnt);
    int xferCycles = 0;

    if (gamePakPtr_ != nullptr)
    {
        eepromRead = gamePakPtr_->EepromAccess(internalSrcAddr_);
        eepromWrite = gamePakPtr_->EepromAccess(internalDestAddr_);
    }

    if (eepromRead || eepromWrite)
    {
        // TODO
    }
    else if (fifoXfer)
    {
        // TODO
    }
    else
    {
        xferCycles = ExecuteNormalXfer();
    }

    if (dmacnt.repeat)
    {
        internalWordCount_ = GetWordCount() & ((channelIndex_ == 3) ? 0xFFFF : 0x3FFF);

        if (internalWordCount_ == 0)
        {
            internalWordCount_ = (channelIndex_ == 3) ? 0x0001'0000 : 0x4000;
        }

        if (dmacnt.destAddrCnt == 3)
        {
            internalDestAddr_ = GetDAD() & ((channelIndex_ == 3) ? 0x0FFF'FFFF : 0x07FF'FFFF);
        }
    }

    if (!dmacnt.repeat || (dmacnt.timing == 0))
    {
        dmacnt.enable = 0;
    }

    SetDMACNT(dmacnt);
    bool enabled = dmacnt.enable;
    std::optional<InterruptType> interrupt = {};

    if (dmacnt.irq)
    {
        interrupt = interruptType_;
    }

    return {xferCycles, enabled, interrupt};
}

XferType DmaChannel::DetermineStartTiming(DMACNT dmacnt) const
{
    auto dmaState = XferType::NO_CHANGE;

    switch (dmacnt.timing)
    {
        case 0:
            dmaState = XferType::IMMEDIATE;
            break;
        case 1:
            dmaState = XferType::VBLANK;
            break;
        case 2:
            dmaState = XferType::HBLANK;
            break;
        case 3:
        {
            if (IsFifoXfer(dmacnt))
            {
                dmaState = (GetDAD() == FIFO_A_ADDR) ? XferType::FIFO_A : XferType::FIFO_B;
            }
            else if (channelIndex_ == 3)
            {
                dmaState = XferType::VIDEO_CAPTURE;
            }

            break;
        }
    }

    return dmaState;
}

bool DmaChannel::IsFifoXfer(DMACNT dmacnt) const
{
    return dmacnt.repeat &&
           ((GetDAD() == FIFO_A_ADDR) || (GetDAD() == FIFO_B_ADDR)) &&
           ((channelIndex_ == 1) || (channelIndex_ == 2));
}

int DmaChannel::ExecuteNormalXfer()
{
    int xferCycles = 0;
    auto dmacnt = GetDMACNT();
    auto length = dmacnt.xferType ? AccessSize::WORD : AccessSize::HALFWORD;
    i8 srcAddrDelta = 0;
    i8 destAddrDelta = 0;

    switch (dmacnt.srcAddrCnt)
    {
        case 0:
            srcAddrDelta = static_cast<i8>(length);
            break;
        case 1:
            srcAddrDelta = -1 * static_cast<i8>(length);
            break;
        case 2:
        case 3:
            break;
    }

    switch (dmacnt.destAddrCnt)
    {
        case 0:
            destAddrDelta = static_cast<i8>(length);
            break;
        case 1:
            destAddrDelta = -1 * static_cast<i8>(length);
            break;
        case 2:
            break;
        case 3:
            destAddrDelta = static_cast<i8>(length);
            break;
    }

    while (internalWordCount_ > 0)
    {
        auto [val, readCycles] = ReadMemory(internalSrcAddr_, length);
        int writeCycles = WriteMemory(internalDestAddr_, val, length);
        xferCycles += readCycles + writeCycles;
        --internalWordCount_;
        internalSrcAddr_ += srcAddrDelta;
        internalDestAddr_ += destAddrDelta;
    }

    return xferCycles;
}
}  // namespace dma
