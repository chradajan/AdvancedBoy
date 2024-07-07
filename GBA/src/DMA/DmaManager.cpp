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

MemReadData DmaManager::ReadReg(Address addr, AccessSize length)
{
    (void)addr;
    (void)length;
    return {ONE_CYCLE, 0, false};
}

CpuCycles DmaManager::WriteReg(Address addr, u32 val, AccessSize length)
{
    (void)addr;
    (void)val;
    (void)length;
    return ONE_CYCLE;
}
}  // namespace dma
