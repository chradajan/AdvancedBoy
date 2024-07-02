#pragma once

#include <memory>
#include <GBA/include/APU/APU.hpp>
#include <GBA/include/BIOS/BIOSManager.hpp>
#include <GBA/include/Cartridge/GamePak.hpp>
#include <GBA/include/DMA/DmaManager.hpp>
#include <GBA/include/Keypad/Keypad.hpp>
#include <GBA/include/PPU/PPU.hpp>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Timers/TimerManager.hpp>
#include <GBA/include/Types.hpp>

/// @brief Class that routes memory reads/writes and owns components connected to the bus.
class Bus
{
public:

private:
    friend class GameBoyAdvance;

    audio::APU apu_;
    bios::BIOSManager biosMgr_;
    std::unique_ptr<cartridge::GamePak> gamePak_;
    cartridge::GamePak gamePak_;
    dma::DmaManager dmaMgr_;
    controller::Keypad keyPad_;
    graphics::PPU ppu_;
    system::SystemControl systemControl_;
    timers::TimerManager timerMgr_;
};
