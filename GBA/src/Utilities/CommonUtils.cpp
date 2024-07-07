#include <GBA/include/Utilities/CommonUtils.hpp>
#include <cstddef>
#include <cstring>
#include <span>
#include <stdexcept>
#include <GBA/include/Types.hpp>

u32 ReadMemoryBlock(std::span<const std::byte> memory, u32 readAddr, u32 baseAddr, AccessSize length)
{
    size_t index = readAddr - baseAddr;
    size_t count = static_cast<size_t>(length);

    if ((index + count) > memory.size())
    {
        throw std::out_of_range("Bad memory read");
    }

    u32 val;
    std::memcpy(&val, &memory[index], count);
    return val;
}

void WriteMemoryBlock(std::span<std::byte> memory, u32 writeAddr, u32 baseAddr, u32 val, AccessSize length)
{
    size_t index = writeAddr - baseAddr;
    size_t count = static_cast<size_t>(length);

    if ((index + count) > memory.size())
    {
        throw std::out_of_range("Bad memory write");
    }

    std::memcpy(&memory[index], &val, count);
}
