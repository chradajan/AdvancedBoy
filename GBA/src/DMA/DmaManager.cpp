#include <GBA/include/DMA/DmaManager.hpp>
#include <array>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <utility>
#include <GBA/include/Cartridge/GamePak.hpp>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Types/Types.hpp>

namespace dma
{
DmaManager::DmaManager(ReadMemCallback readMem,
                       WriteMemCallback writeMem,
                       EventScheduler& scheduler,
                       SystemControl& systemControl) :
    dmaChannels_{
        DmaChannel(0, InterruptType::DMA0, readMem, writeMem),
        DmaChannel(1, InterruptType::DMA1, readMem, writeMem),
        DmaChannel(2, InterruptType::DMA2, readMem, writeMem),
        DmaChannel(3, InterruptType::DMA3, readMem, writeMem)
    },
    scheduler_(scheduler),
    systemControl_(systemControl)
{
    vBlank_.fill(false);
    hBlank_.fill(false);
    fifoA_.fill(false);
    fifoB_.fill(false);
    videoCapture_.fill(false);
    active_ = false;

    scheduler_.RegisterEvent(EventType::DmaComplete, std::bind(&DmaManager::EndDma, this, std::placeholders::_1));
}

void DmaManager::ConnectGamePak(cartridge::GamePak* gamePakPtr)
{
    for (DmaChannel& channel : dmaChannels_)
    {
        channel.ConnectGamePak(gamePakPtr);
    }
}

MemReadData DmaManager::ReadReg(u32 addr, AccessSize length)
{
    if (addr <= DMA0_ADDR_MAX)
    {
        return dmaChannels_[0].ReadReg(addr, length);
    }
    else if (addr <= DMA1_ADDR_MAX)
    {
        return dmaChannels_[1].ReadReg(addr, length);
    }
    else if (addr <= DMA2_ADDR_MAX)
    {
        return dmaChannels_[2].ReadReg(addr, length);
    }
    else if (addr <= DMA3_ADDR_MAX)
    {
        return dmaChannels_[3].ReadReg(addr, length);
    }

    throw std::runtime_error("Read invalid DmaChannel address");
}

int DmaManager::WriteReg(u32 addr, u32 val, AccessSize length)
{
    u8 index;

    if (addr <= DMA0_ADDR_MAX)
    {
        index = 0;
    }
    else if (addr <= DMA1_ADDR_MAX)
    {
        index = 1;
    }
    else if (addr <= DMA2_ADDR_MAX)
    {
        index = 2;
    }
    else if (addr <= DMA3_ADDR_MAX)
    {
        index = 3;
    }
    else
    {
        throw std::runtime_error("Wrote invalid DmaChannel address");
    }

    XferType dmaState = dmaChannels_[index].WriteReg(addr, val, length);

    if (dmaState != XferType::NO_CHANGE)
    {
        vBlank_[index] = false;
        hBlank_[index] = false;
        fifoA_[index] = false;
        fifoB_[index] = false;
        videoCapture_[index] = false;

        switch (dmaState)
        {
            case XferType::NO_CHANGE:
            case XferType::DISABLED:
                break;
            case XferType::IMMEDIATE:
                HandleDmaEvents(dmaChannels_[index].Execute());
                break;
            case XferType::VBLANK:
                vBlank_[index] = true;
                break;
            case XferType::HBLANK:
                hBlank_[index] = true;
                break;
            case XferType::FIFO_A:
                fifoA_[index] = true;
                break;
            case XferType::FIFO_B:
                fifoB_[index] = true;
                break;
            case XferType::VIDEO_CAPTURE:
                videoCapture_[index] = true;
                break;
        }
    }

    return 1;
}

void DmaManager::HandleDmaEvents(ExecuteResult result)
{
    if (result.cycles == 0)
    {
        return;
    }

    auto currentXferCycles = scheduler_.UnscheduleEvent(EventType::DmaComplete);

    if (currentXferCycles.has_value())
    {
        result.cycles += *currentXferCycles;
    }

    scheduler_.ScheduleEvent(EventType::DmaComplete, result.cycles);
    active_ = true;

    if (result.interrupt.has_value())
    {
        systemControl_.RequestInterrupt(*result.interrupt);
    }
}

void DmaManager::CheckSpecialTiming(std::array<bool, 4>& enabledChannels)
{
    for (u8 i = 0; i < 4; ++i)
    {
        if (enabledChannels[i])
        {
            auto result = dmaChannels_[i].Execute();
            enabledChannels[i] = result.enabled;
            HandleDmaEvents(result);
        }
    }
}
}  // namespace dma
