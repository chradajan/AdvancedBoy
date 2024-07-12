#include <GBA/include/Cartridge/EEPROM.hpp>
#include <filesystem>
#include <GBA/include/Memory/MemoryMap.hpp>
#include <GBA/include/Types.hpp>

namespace cartridge
{
EEPROM::EEPROM(fs::path savePath, bool largeCart) : largeCart_(largeCart)
{
    savePath_ = savePath;
}

bool EEPROM::IsBackupMediaAccess(u32 addr) const
{
    if (largeCart_)
    {
        return ((EEPROM_LARGE_CART_ADDR_MIN <= addr) && (addr <= EEPROM_ADDR_MAX));
    }

    return ((EEPROM_SMALL_CART_ADDR_MIN <= addr) && (addr <= EEPROM_ADDR_MAX));
}

MemReadData EEPROM::ReadMem(u32 addr, AccessSize length)
{
    (void)addr;
    (void)length;
    return {1, 0xFF, false};
}

int EEPROM::WriteMem(u32 addr, u32 val, AccessSize length)
{
    (void)addr;
    (void)val;
    (void)length;
    return 1;
}

void EEPROM::Save() const
{
}
}  // namespace cartridge
