#include <GBA/include/Debug/SystemControlDebugger.hpp>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>

namespace debug
{
u32 SystemControlDebugger::ReadRegister(u32 addr, AccessSize length) const
{
    switch (addr)
    {
        case INT_WAITCNT_ADDR_MIN ... INT_WAITCNT_ADDR_MAX:
            return ReadMemoryBlock(systemControl_.interruptAndWaitcntRegisters_, addr, INT_WAITCNT_ADDR_MIN, length);
        case POSTFLG_HALTCNT_ADDR_MIN ... POSTFLG_HALTCNT_ADDR_MAX:
            return ReadMemoryBlock(systemControl_.postFlgAndHaltcntRegisters_, addr, POSTFLG_HALTCNT_ADDR_MIN, length);
        case INTERNAL_MEM_CONTROL_ADDR_MIN ... INTERNAL_MEM_CONTROL_ADDR_MAX:
            return ReadMemoryBlock(systemControl_.memoryControlRegisters_, addr, INTERNAL_MEM_CONTROL_ADDR_MIN, length);
        default:
            break;
    }

    return 0;
}
}
