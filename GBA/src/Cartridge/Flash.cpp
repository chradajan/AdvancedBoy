#include <GBA/include/Cartridge/Flash.hpp>
#include <filesystem>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/Types.hpp>

namespace cartridge
{
Flash::Flash(fs::path savePath)
{
    savePath_ = savePath;
}

bool Flash::IsBackupMediaAccess(u32 addr) const
{
    return (SRAM_ADDR_MIN <= addr) && (addr <= SRAM_ADDR_MAX);
}

MemReadData Flash::ReadMem(u32 addr, AccessSize length)
{
    (void)addr;
    (void)length;
    return {1, 0xFF, false};
}

int Flash::WriteMem(u32 addr, u32 val, AccessSize length)
{
    (void)addr;
    (void)val;
    (void)length;
    return 1;
}

void Flash::Save() const
{
}
}  // namespace cartridge
