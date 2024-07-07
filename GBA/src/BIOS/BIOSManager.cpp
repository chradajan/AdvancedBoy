#include <filesystem>
#include <GBA/include/BIOS/BIOSManager.hpp>
#include <GBA/include/Types.hpp>
#include <GBA/include/Utilities/Functor.hpp>

BIOSManager::BIOSManager(fs::path biosPath, GetPCCallback getPC) : GetPC(getPC)
{
    (void)biosPath;
}

MemReadData BIOSManager::ReadMem(u32 addr, AccessSize length)
{
    (void)addr;
    (void)length;
    return {1, 0, false};
}

int BIOSManager::WriteMem(u32 addr, u32 val, AccessSize length)
{
    (void)addr;
    (void)length;
    (void)val;
    return 1;
}
