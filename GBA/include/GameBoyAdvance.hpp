#pragma once

#include <memory>
#include <GBA/include/CPU/ARM7TDMI.hpp>
#include <GBA/include/Memory/Bus.hpp>
#include <GBA/include/PPU/PPU.hpp>
#include <GBA/include/System/EventScheduler.hpp>
#include <GBA/include/Types.hpp>

/// @brief Represents a single GBA.
class GameBoyAdvance
{
public:
private:
    cpu::ARM7TDMI cpu_;
    Bus bus_;
    EventScheduler scheduler_;
};
