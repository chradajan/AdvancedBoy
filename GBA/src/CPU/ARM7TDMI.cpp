#include <GBA/include/CPU/ARM7TDMI.hpp>
#include <GBA/include/Types.hpp>

namespace cpu
{
ARM7TDMI::ARM7TDMI(ReadMemCallback readMem, WriteMemCallback writeMem) : ReadMemory(readMem), WriteMemory(writeMem)
{
}
}  // namespace cpu
