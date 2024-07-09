#include <GBA/include/Cartridge/GamePak.hpp>
#include <GBA/include/Types.hpp>

namespace cartridge
{
MemReadData GamePak::ReadMem(u32 addr, AccessSize length)
{
    (void)addr;
    (void)length;
    return {1, 0, false};
}

int GamePak::WriteMem(u32 addr, u32 val, AccessSize length)
{
    (void)addr;
    (void)length;
    (void)val;
    return 1;
}

MemReadData GamePak::ReadUnloadedGamePakMem(u32 addr, AccessSize length)
{
    u32 val = 0;
    size_t count = static_cast<size_t>(length);
    u32 currAddr = addr + count - 1;

    while (count > 0)
    {
        val <<= 8;
        val |= ((currAddr / 2) & 0x0000'FFFF);
        --count;
    }

    return {1, val, false};
}
}  // namespace cartridge
