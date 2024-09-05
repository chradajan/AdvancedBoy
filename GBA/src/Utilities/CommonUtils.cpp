#include <GBA/include/Utilities/CommonUtils.hpp>
#include <bit>
#include <cstddef>
#include <cstring>
#include <span>
#include <stdexcept>
#include <GBA/include/Types/Types.hpp>

u32 ReadMemoryBlock(std::span<const std::byte> memory, u32 readAddr, u32 baseAddr, AccessSize length)
{
    size_t index = readAddr - baseAddr;
    size_t count = static_cast<size_t>(length);

    if ((index + count) > memory.size())
    {
        throw std::out_of_range("Bad memory read");
    }

    u32 val = 0;

    switch (length)
    {
        case AccessSize::BYTE:
            std::memcpy(&val, &memory[index], sizeof(u8));
            break;
        case AccessSize::HALFWORD:
            std::memcpy(&val, &memory[index], sizeof(u16));
            break;
        case AccessSize::WORD:
            std::memcpy(&val, &memory[index], sizeof(u32));
            break;
        case AccessSize::INVALID:
            throw std::runtime_error("Illegal memory access size on read");
    }

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

    switch (length)
    {
        case AccessSize::BYTE:
            std::memcpy(&memory[index], &val, sizeof(u8));
            break;
        case AccessSize::HALFWORD:
            std::memcpy(&memory[index], &val, sizeof(u16));
            break;
        case AccessSize::WORD:
            std::memcpy(&memory[index], &val, sizeof(u32));
            break;
        case AccessSize::INVALID:
            throw std::runtime_error("Illegal memory access size on write");
    }
}

u32 StandardMirroredAddress(u32 addr, u32 min, u32 max)
{
    u32 exclusiveMax = max + 1;
    u32 regionSize = exclusiveMax - min;
    addr = ((addr - exclusiveMax) % regionSize) + min;
    return addr;
}

u32 Read8BitBus(u8 byte, AccessSize length)
{
    u32 val = byte;

    if (length == AccessSize::HALFWORD)
    {
        val *= 0x0101;
    }
    else if (length == AccessSize::WORD)
    {
        val *= 0x0101'0101;
    }

    return val;
}

u8 Write8BitBus(u32 addr, u32 val)
{
    return std::rotr(val, (addr & 0x03) * 8) & U8_MAX;
}
