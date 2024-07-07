#include <filesystem>
#include <GBA/include/BIOS/BIOSManager.hpp>
#include <GBA/include/Types.hpp>
#include <GBA/include/Utilities/Functor.hpp>

BIOSManager::BIOSManager(fs::path biosPath, GetPCCallback getPC) : GetPC(getPC)
{
    (void)biosPath;
}

MemReadData BIOSManager::ReadMem(Address addr, AccessSize length)
{
    (void)addr;
    (void)length;
    return {ONE_CYCLE, 0, false};
}

CpuCycles BIOSManager::WriteMem(Address addr, u32 val, AccessSize length)
{
    (void)addr;
    (void)length;
    (void)val;
    return ONE_CYCLE;
}
