#include <GBA/include/BIOS/BIOSManager.hpp>
#include <filesystem>
#include <fstream>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/Types/Types.hpp>
#include <GBA/include/Utilities/CommonUtils.hpp>
#include <GBA/include/Utilities/Functor.hpp>

BIOSManager::BIOSManager(fs::path biosPath, GetPCCallback getPC) : GetPC(getPC)
{
    lastSuccessfulFetch_ = 0;
    biosLoaded_ = false;

    if (biosPath.empty() || !fs::exists(biosPath) || !fs::is_regular_file(biosPath))
    {
        return;
    }

    size_t fileSizeInBytes = fs::file_size(biosPath);

    if (fileSizeInBytes != biosROM_.size())
    {
        return;
    }

    std::ifstream bios(biosPath, std::ios::binary);

    if (bios.fail())
    {
        return;
    }

    bios.read(reinterpret_cast<char*>(biosROM_.data()), fileSizeInBytes);
    biosLoaded_ = true;
}

MemReadData BIOSManager::ReadMem(u32 addr, AccessSize length)
{
    int cycles = 1;

    if (addr > BIOS_ADDR_MAX)
    {
        return {cycles, 0, true};
    }

    if (GetPC() > BIOS_ADDR_MAX)
    {
        return {cycles, lastSuccessfulFetch_, false};
    }

    lastSuccessfulFetch_ = ReadMemoryBlock(biosROM_, addr, BIOS_ADDR_MIN, length);
    return {cycles, lastSuccessfulFetch_, false};
}
