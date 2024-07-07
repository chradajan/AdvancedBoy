#include <GBA/include/DMA/DmaManager.hpp>
#include <array>
#include <cstddef>
#include <utility>
#include <GBA/include/System/SystemControl.hpp>
#include <GBA/include/Types.hpp>

namespace dma
{
DmaManager::DmaManager(ReadMemCallback readMem, WriteMemCallback writeMem, SystemControl& systemControl) :
    ReadMemory(readMem),
    WriteMemory(writeMem),
    systemControl_(systemControl)
{
    registers_.fill(std::byte{0});
}

MemReadData DmaManager::ReadReg(u32 addr, AccessSize length)
{
    (void)addr;
    (void)length;
    return {1, 0, false};
}

int DmaManager::WriteReg(u32 addr, u32 val, AccessSize length)
{
    (void)addr;
    (void)val;
    (void)length;
    return 1;
}
}  // namespace dma
