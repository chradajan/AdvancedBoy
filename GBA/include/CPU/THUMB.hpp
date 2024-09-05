#pragma once

#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/Types/Types.hpp>

namespace cpu::thumb
{ 
///---------------------------------------------------------------------------------------------------------------------------------
/// SoftwareInterrupt
///---------------------------------------------------------------------------------------------------------------------------------

struct SoftwareInterrupt
{
    static constexpr u16 FORMAT         = 0b1101'1111'0000'0000;
    static constexpr u16 FORMAT_MASK    = 0b1111'1111'0000'0000;
    static constexpr bool IsInstanceOf(u16 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u16 Value8  : 8;
        u16         : 8;
    };
};

static_assert(sizeof(SoftwareInterrupt::Flags) == sizeof(u16), "SoftwareInterrupt::Flags must be 2 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// UnconditionalBranch
///---------------------------------------------------------------------------------------------------------------------------------

struct UnconditionalBranch
{
    static constexpr u16 FORMAT         = 0b1110'0000'0000'0000;
    static constexpr u16 FORMAT_MASK    = 0b1111'1000'0000'0000;
    static constexpr bool IsInstanceOf(u16 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u16 Offset11    : 11;
        u16             : 5;
    };
};

static_assert(sizeof(UnconditionalBranch::Flags) == sizeof(u16), "UnconditionalBranch::Flags must be 2 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// ConditionalBranch
///---------------------------------------------------------------------------------------------------------------------------------

struct ConditionalBranch
{
    static constexpr u16 FORMAT         = 0b1101'0000'0000'0000;
    static constexpr u16 FORMAT_MASK    = 0b1111'0000'0000'0000;
    static constexpr bool IsInstanceOf(u16 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u16 SOffset8    : 8;
        u16 Cond        : 4;
        u16             : 4;
    };
};

static_assert(sizeof(ConditionalBranch::Flags) == sizeof(u16), "ConditionalBranch::Flags must be 2 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// MultipleLoadStore
///---------------------------------------------------------------------------------------------------------------------------------

struct MultipleLoadStore
{
    static constexpr u16 FORMAT         = 0b1100'0000'0000'0000;
    static constexpr u16 FORMAT_MASK    = 0b1111'0000'0000'0000;
    static constexpr bool IsInstanceOf(u16 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u16 Rlist   : 8;
        u16 Rb      : 3;
        u16 L       : 1;
        u16         : 4;
    };
};

static_assert(sizeof(MultipleLoadStore::Flags) == sizeof(u16), "MultipleLoadStore::Flags must be 2 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// LongBranchWithLink
///---------------------------------------------------------------------------------------------------------------------------------

struct LongBranchWithLink
{
    static constexpr u16 FORMAT         = 0b1111'0000'0000'0000;
    static constexpr u16 FORMAT_MASK    = 0b1111'0000'0000'0000;
    static constexpr bool IsInstanceOf(u16 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u16 Offset  : 11;
        u16 H       : 1;
        u16         : 4;
    };
};

static_assert(sizeof(LongBranchWithLink::Flags) == sizeof(u16), "LongBranchWithLink::Flags must be 2 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// AddOffsetToStackPointer
///---------------------------------------------------------------------------------------------------------------------------------

struct AddOffsetToStackPointer
{
    static constexpr u16 FORMAT         = 0b1011'0000'0000'0000;
    static constexpr u16 FORMAT_MASK    = 0b1111'1111'0000'0000;
    static constexpr bool IsInstanceOf(u16 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u16 SWord7  : 7;
        u16 S       : 1;
        u16         : 8;
    };
};

static_assert(sizeof(AddOffsetToStackPointer::Flags) == sizeof(u16), "AddOffsetToStackPointer::Flags must be 2 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// PushPopRegisters
///---------------------------------------------------------------------------------------------------------------------------------

struct PushPopRegisters
{
    static constexpr u16 FORMAT         = 0b1011'0100'0000'0000;
    static constexpr u16 FORMAT_MASK    = 0b1111'0110'0000'0000;
    static constexpr bool IsInstanceOf(u16 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u16 Rlist   : 8;
        u16 R       : 1;
        u16         : 2;
        u16 L       : 1;
        u16         : 4;
    };
};

static_assert(sizeof(PushPopRegisters::Flags) == sizeof(u16), "PushPopRegisters::Flags must be 2 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// LoadStoreHalfword
///---------------------------------------------------------------------------------------------------------------------------------

struct LoadStoreHalfword
{
    static constexpr u16 FORMAT         = 0b1000'0000'0000'0000;
    static constexpr u16 FORMAT_MASK    = 0b1111'0000'0000'0000;
    static constexpr bool IsInstanceOf(u16 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u16 Rd          : 3;
        u16 Rb          : 3;
        u16  Offset5    : 5;
        u16 L           : 1;
        u16             : 4;
    };
};

static_assert(sizeof(LoadStoreHalfword::Flags) == sizeof(u16), "LoadStoreHalfword::Flags must be 2 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// SPRelativeLoadStore
///---------------------------------------------------------------------------------------------------------------------------------

struct SPRelativeLoadStore
{
    static constexpr u16 FORMAT         = 0b1001'0000'0000'0000;
    static constexpr u16 FORMAT_MASK    = 0b1111'0000'0000'0000;
    static constexpr bool IsInstanceOf(u16 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u16 Word8   : 8;
        u16 Rd      : 3;
        u16 L       : 1;
        u16         : 4;
    };
};

static_assert(sizeof(SPRelativeLoadStore::Flags) == sizeof(u16), "SPRelativeLoadStore::Flags must be 2 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// LoadAddress
///---------------------------------------------------------------------------------------------------------------------------------

struct LoadAddress
{
    static constexpr u16 FORMAT         = 0b1010'0000'0000'0000;
    static constexpr u16 FORMAT_MASK    = 0b1111'0000'0000'0000;
    static constexpr bool IsInstanceOf(u16 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u16 Word8   : 8;
        u16 Rd      : 3;
        u16 SP      : 1;
        u16         : 4;
    };
};

static_assert(sizeof(LoadAddress::Flags) == sizeof(u16), "LoadAddress::Flags must be 2 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// LoadStoreWithImmOffset
///---------------------------------------------------------------------------------------------------------------------------------

struct LoadStoreWithImmOffset
{
    static constexpr u16 FORMAT         = 0b0110'0000'0000'0000;
    static constexpr u16 FORMAT_MASK    = 0b1110'0000'0000'0000;
    static constexpr bool IsInstanceOf(u16 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u16 Rd      : 3;
        u16 Rb      : 3;
        u16 Offset5 : 5;
        u16 L       : 1;
        u16 B       : 1;
        u16         : 3;
    };
};

static_assert(sizeof(LoadStoreWithImmOffset::Flags) == sizeof(u16), "LoadStoreWithImmOffset::Flags must be 2 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// LoadStoreWithRegOffset
///---------------------------------------------------------------------------------------------------------------------------------

struct LoadStoreWithRegOffset
{
    static constexpr u16 FORMAT         = 0b0101'0000'0000'0000;
    static constexpr u16 FORMAT_MASK    = 0b1111'0010'0000'0000;
    static constexpr bool IsInstanceOf(u16 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u16 Rd  : 3;
        u16 Rb  : 3;
        u16 Ro  : 3;
        u16     : 1;
        u16 B   : 1;
        u16 L   : 1;
        u16     : 4;
    };
};

static_assert(sizeof(LoadStoreWithRegOffset::Flags) == sizeof(u16), "LoadStoreWithRegOffset::Flags must be 2 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// LoadStoreSignExtendedByteHalfword
///---------------------------------------------------------------------------------------------------------------------------------

struct LoadStoreSignExtendedByteHalfword
{
    static constexpr u16 FORMAT         = 0b0101'0010'0000'0000;
    static constexpr u16 FORMAT_MASK    = 0b1111'0010'0000'0000;
    static constexpr bool IsInstanceOf(u16 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u16 Rd  : 3;
        u16 Rb  : 3;
        u16 Ro  : 3;
        u16     : 1;
        u16 S   : 1;
        u16 H   : 1;
        u16     : 4;
    };
};

static_assert(sizeof(LoadStoreSignExtendedByteHalfword::Flags) == sizeof(u16), "LoadStoreSignExtendedByteHalfword::Flags must be 2 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// PCRelativeLoad
///---------------------------------------------------------------------------------------------------------------------------------

struct PCRelativeLoad
{
    static constexpr u16 FORMAT         = 0b0100'1000'0000'0000;
    static constexpr u16 FORMAT_MASK    = 0b1111'1000'0000'0000;
    static constexpr bool IsInstanceOf(u16 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u16 Word8   : 8;
        u16 Rd      : 3;
        u16         : 5;
    };
};

static_assert(sizeof(PCRelativeLoad::Flags) == sizeof(u16), "PCRelativeLoad::Flags must be 2 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// HiRegisterOperationsBranchExchange
///---------------------------------------------------------------------------------------------------------------------------------

struct HiRegisterOperationsBranchExchange
{
    static constexpr u16 FORMAT         = 0b0100'0100'0000'0000;
    static constexpr u16 FORMAT_MASK    = 0b1111'1100'0000'0000;
    static constexpr bool IsInstanceOf(u16 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u16 RdHd    : 3;
        u16 RsHs    : 3;
        u16 H2      : 1;
        u16 H1      : 1;
        u16 Op      : 2;
        u16         : 6;
    };
};

static_assert(sizeof(HiRegisterOperationsBranchExchange::Flags) == sizeof(u16), "HiRegisterOperationsBranchExchange::Flags must be 2 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// ALUOperations
///---------------------------------------------------------------------------------------------------------------------------------

struct ALUOperations
{
    static constexpr u16 FORMAT         = 0b0100'0000'0000'0000;
    static constexpr u16 FORMAT_MASK    = 0b1111'1100'0000'0000;
    static constexpr bool IsInstanceOf(u16 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u16 Rd  : 3;
        u16 Rs  : 3;
        u16 Op  : 4;
        u16     : 6;
    };
};

static_assert(sizeof(ALUOperations::Flags) == sizeof(u16), "ALUOperations::Flags must be 2 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// MoveCompareAddSubtractImmediate
///---------------------------------------------------------------------------------------------------------------------------------

struct MoveCompareAddSubtractImmediate
{
    static constexpr u16 FORMAT         = 0b0010'0000'0000'0000;
    static constexpr u16 FORMAT_MASK    = 0b1110'0000'0000'0000;
    static constexpr bool IsInstanceOf(u16 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u16 Offset8 : 8;
        u16 Rd      : 3;
        u16 Op      : 2;
        u16         : 3;
    };
};

static_assert(sizeof(MoveCompareAddSubtractImmediate::Flags) == sizeof(u16), "MoveCompareAddSubtractImmediate::Flags must be 2 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// AddSubtract
///---------------------------------------------------------------------------------------------------------------------------------

struct AddSubtract
{
    static constexpr u16 FORMAT         = 0b0001'1000'0000'0000;
    static constexpr u16 FORMAT_MASK    = 0b1111'1000'0000'0000;
    static constexpr bool IsInstanceOf(u16 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u16 Rd          : 3;
        u16 Rs          : 3;
        u16 RnOffset3   : 3;
        u16 Op          : 1;
        u16 I           : 1;
        u16             : 5;
    };
};

static_assert(sizeof(AddSubtract::Flags) == sizeof(u16), "AddSubtract::Flags must be 2 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// MoveShiftedRegister
///---------------------------------------------------------------------------------------------------------------------------------

struct MoveShiftedRegister
{
    static constexpr u16 FORMAT         = 0b0000'0000'0000'0000;
    static constexpr u16 FORMAT_MASK    = 0b1110'0000'0000'0000;
    static constexpr bool IsInstanceOf(u16 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u16 Rd      : 3;
        u16 Rs      : 3;
        u16 Offset5 : 5;
        u16 Op      : 2;
        u16         : 3;
    };
};

static_assert(sizeof(MoveShiftedRegister::Flags) == sizeof(u16), "MoveShiftedRegister::Flags must be 2 bytes");
}  // namespace cpu::thumb
