#include <GBA/include/Cartridge/SRAM.hpp>
#include <filesystem>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/Types.hpp>

namespace cartridge
{
SRAM::SRAM(fs::path savePath)
{
    savePath_ = savePath;
}

bool SRAM::IsBackupMediaAccess(u32 addr) const
{
    return (SRAM_ADDR_MIN <= addr) && (addr <= SRAM_ADDR_MAX);
}

MemReadData SRAM::ReadMem(u32 addr, AccessSize length)
{
    (void)addr;
    (void)length;
    return {1, 0xFF, false};
}

int SRAM::WriteMem(u32 addr, u32 val, AccessSize length)
{
    (void)addr;
    (void)val;
    (void)length;
    return 1;
}

void SRAM::Save() const
{
}
}  // namespace cartridge
