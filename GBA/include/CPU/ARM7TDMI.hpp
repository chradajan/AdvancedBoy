#pragma once

#include <utility>
#include <GBA/include/Types.hpp>
#include <GBA/include/Utilities/Functor.hpp>

class GameBoyAdvance;

namespace cpu
{
/// @brief ARM7TDMI CPU.
class ARM7TDMI
{
    using ReadMemCallback = MemberFunctor<std::pair<u32, CpuCycles> (GameBoyAdvance::*)(Address, AccessSize)>;
    using WriteMemCallback = MemberFunctor<CpuCycles (GameBoyAdvance::*)(Address, u32, AccessSize)>;

public:
    ARM7TDMI() = delete;
    ARM7TDMI(ARM7TDMI const&) = delete;
    ARM7TDMI& operator=(ARM7TDMI const&) = delete;
    ARM7TDMI(ARM7TDMI&&) = delete;
    ARM7TDMI& operator=(ARM7TDMI&&) = delete;

    /// @brief Initialize an ARM7TDMI CPU.
    /// @param ReadMem Callback function to access bus read functionality.
    /// @param WriteMem Callback function to access bus write functionality.
    explicit ARM7TDMI(ReadMemCallback readMem, WriteMemCallback writeMem);

    /// @brief TODO Get the current PC value that code is executing from.
    /// @return Current r15 value.
    u32 GetPC() { return 0; }

private:
    ReadMemCallback ReadMemory;
    WriteMemCallback WriteMemory;
};
}  // namespace cpu
