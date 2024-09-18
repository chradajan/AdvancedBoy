#include <GBA/include/Debug/APUDebugger.hpp>
#include <GBA/include/APU/APU.hpp>
#include <GBA/include/APU/Channel1.hpp>
#include <GBA/include/APU/Channel2.hpp>
#include <GBA/include/APU/Channel4.hpp>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>
#include <GBA/include/Utilities/Types.hpp>

namespace debug
{
u32 APUDebugger::ReadRegister(u32 addr, AccessSize length) const
{
    switch (addr)
    {
        case CHANNEL_1_ADDR_MIN ... CHANNEL_1_ADDR_MAX:
            return ReadMemoryBlock(apu_.channel1_.registers_, addr, CHANNEL_1_ADDR_MIN, length);
        case CHANNEL_2_ADDR_MIN ... CHANNEL_2_ADDR_MAX:
            return ReadMemoryBlock(apu_.channel2_.registers_, addr, CHANNEL_2_ADDR_MIN, length);
        case CHANNEL_3_ADDR_MIN ... CHANNEL_3_ADDR_MAX:
            return 0;
        case CHANNEL_4_ADDR_MIN ... CHANNEL_4_ADDR_MAX:
            return ReadMemoryBlock(apu_.channel4_.registers_, addr, CHANNEL_4_ADDR_MIN, length);
        case APU_CONTROL_ADDR_MIN ... APU_CONTROL_ADDR_MAX:
            return ReadMemoryBlock(apu_.registers_, addr, APU_CONTROL_ADDR_MIN, length);
        case WAVE_RAM_ADDR_MIN ... WAVE_RAM_ADDR_MAX:
            return 0;
        default:
            break;
    }

    return 0;
}
}  // namespace debug
