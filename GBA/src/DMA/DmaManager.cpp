#include <GBA/include/DMA/DmaManager.hpp>
#include <GBA/include/Types.hpp>

namespace dma
{
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
