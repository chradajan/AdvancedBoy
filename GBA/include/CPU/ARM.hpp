#pragma once

#include <GBA/include/CPU/CpuTypes.hpp>
#include <GBA/include/Types.hpp>

namespace cpu::arm
{
///---------------------------------------------------------------------------------------------------------------------------------
/// BranchAndExchange
///---------------------------------------------------------------------------------------------------------------------------------

struct BranchAndExchange
{
    static constexpr u32 FORMAT         = 0b0000'0001'0010'1111'1111'1111'0001'0000;
    static constexpr u32 FORMAT_MASK    = 0b0000'1111'1111'1111'1111'1111'1111'0000;
    static constexpr bool IsInstanceOf(u32 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u32 Rn      : 4;
        u32         : 24;
        u32 Cond    : 4;
    };
};

static_assert(sizeof(BranchAndExchange::Flags) == sizeof(u32), "BranchAndExchange::Flags must be 4 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// BlockDataTransfer
///---------------------------------------------------------------------------------------------------------------------------------

struct BlockDataTransfer
{
    static constexpr u32 FORMAT         = 0b0000'1000'0000'0000'0000'0000'0000'0000;
    static constexpr u32 FORMAT_MASK    = 0b0000'1110'0000'0000'0000'0000'0000'0000;
    static constexpr bool IsInstanceOf(u32 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u32 RegisterList    : 16;
        u32 Rn              : 4;
        u32 L               : 1;
        u32 W               : 1;
        u32 S               : 1;
        u32 U               : 1;
        u32 P               : 1;
        u32                 : 3;
        u32 Cond            : 4;
    };
};

static_assert(sizeof(BlockDataTransfer::Flags) == sizeof(u32), "BlockDataTransfer::Flags must be 4 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// Branch
///---------------------------------------------------------------------------------------------------------------------------------

struct Branch
{
    static constexpr u32 FORMAT         = 0b0000'1010'0000'0000'0000'0000'0000'0000;
    static constexpr u32 FORMAT_MASK    = 0b0000'1110'0000'0000'0000'0000'0000'0000;
    static constexpr bool IsInstanceOf(u32 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u32 Offset : 24;
        u32 L      : 1;
        u32        : 3;
        u32 Cond   : 4;
    };
};

static_assert(sizeof(Branch::Flags) == sizeof(u32), "Branch::Flags must be 4 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// SoftwareInterrupt
///---------------------------------------------------------------------------------------------------------------------------------

struct SoftwareInterrupt
{
    static constexpr u32 FORMAT         = 0b0000'1111'0000'0000'0000'0000'0000'0000;
    static constexpr u32 FORMAT_MASK    = 0b0000'1111'0000'0000'0000'0000'0000'0000;
    static constexpr bool IsInstanceOf(u32 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u32 CommentField    : 24;
        u32                 : 4;
        u32 Cond            : 4;
    };
};

static_assert(sizeof(SoftwareInterrupt::Flags) == sizeof(u32), "SoftwareInterrupt::Flags must be 4 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// Undefined
///---------------------------------------------------------------------------------------------------------------------------------

struct Undefined
{
    static constexpr u32 FORMAT         = 0b0000'0110'0000'0000'0000'0000'0001'0000;
    static constexpr u32 FORMAT_MASK    = 0b0000'1110'0000'0000'0000'0000'0001'0000;
    static constexpr bool IsInstanceOf(u32 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u32         : 28;
        u32 Cond    : 4;
    };
};

static_assert(sizeof(Undefined::Flags) == sizeof(u32), "Undefined::Flags must be 4 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// SingleDataTransfer
///---------------------------------------------------------------------------------------------------------------------------------

struct SingleDataTransfer
{
    static constexpr u32 FORMAT         = 0b0000'0100'0000'0000'0000'0000'0000'0000;
    static constexpr u32 FORMAT_MASK    = 0b0000'1100'0000'0000'0000'0000'0000'0000;
    static constexpr bool IsInstanceOf(u32 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u32         : 12;
        u32 Rd      : 4;
        u32 Rn      : 4;
        u32 L       : 1;
        u32 W       : 1;
        u32 B       : 1;
        u32 U       : 1;
        u32 P       : 1;
        u32 I       : 1;
        u32         : 2;
        u32 Cond    : 4;
    };

    struct ImmOffset
    {
        u32 Imm : 12;
        u32     : 20;
    };

    struct RegOffset
    {
        u32 Rm          : 4;
        u32             : 1;
        u32 ShiftType   : 2;
        u32 ShiftAmount : 5;
        u32             : 20;
    };
};

static_assert(sizeof(SingleDataTransfer::Flags) == sizeof(u32), "SingleDataTransfer::Flags must be 4 bytes");
static_assert(sizeof(SingleDataTransfer::ImmOffset) == sizeof(u32), "SingleDataTransfer::ImmOffset must be 4 bytes");
static_assert(sizeof(SingleDataTransfer::RegOffset) == sizeof(u32), "SingleDataTransfer::RegOffset must be 4 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// SingleDataSwap
///---------------------------------------------------------------------------------------------------------------------------------

struct SingleDataSwap
{
    static constexpr u32 FORMAT         = 0b0000'0001'0000'0000'0000'0000'1001'0000;
    static constexpr u32 FORMAT_MASK    = 0b0000'1111'1000'0000'0000'1111'1111'0000;
    static constexpr bool IsInstanceOf(u32 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u32 Rm      : 4;
        u32         : 8;
        u32 Rd      : 4;
        u32 Rn      : 4;
        u32         : 2;
        u32 B       : 1;
        u32         : 5;
        u32 Cond    : 4;
    };
};

static_assert(sizeof(SingleDataSwap::Flags) == sizeof(u32), "SingleDataSwap::Flags must be 4 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// Multiply
///---------------------------------------------------------------------------------------------------------------------------------

struct Multiply
{
    static constexpr u32 FORMAT         = 0b0000'0000'0000'0000'0000'0000'1001'0000;
    static constexpr u32 FORMAT_MASK    = 0b0000'1111'1000'0000'0000'0000'1111'0000;
    static constexpr bool IsInstanceOf(u32 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u32 Rm      : 4;
        u32         : 4;
        u32 Rs      : 4;
        u32 Rn      : 4;
        u32 Rd      : 4;
        u32 S       : 1;
        u32 A       : 1;
        u32         : 6;
        u32 Cond    : 4;
    };
};

static_assert(sizeof(Multiply::Flags) == sizeof(u32), "Multiply::Flags must be 4 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// MultiplyLong
///---------------------------------------------------------------------------------------------------------------------------------

struct MultiplyLong
{
    static constexpr u32 FORMAT         = 0b0000'0000'1000'0000'0000'0000'1001'0000;
    static constexpr u32 FORMAT_MASK    = 0b0000'1111'1000'0000'0000'0000'1111'0000;
    static constexpr bool IsInstanceOf(u32 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u32 Rm      : 4;
        u32         : 4;
        u32 Rs      : 4;
        u32 RdLo    : 4;
        u32 RdHi    : 4;
        u32 S       : 1;
        u32 A       : 1;
        u32 U       : 1;
        u32         : 5;
        u32 Cond    : 4;
    };
};

static_assert(sizeof(MultiplyLong::Flags) == sizeof(u32), "MultiplyLong::Flags must be 4 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// HalfwordDataTransferRegOffset
///---------------------------------------------------------------------------------------------------------------------------------

struct HalfwordDataTransferRegOffset
{
    static constexpr u32 FORMAT         = 0b0000'0000'0000'0000'0000'0000'1001'0000;
    static constexpr u32 FORMAT_MASK    = 0b0000'1110'0100'0000'0000'1111'1001'0000;
    static constexpr bool IsInstanceOf(u32 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u32 Rm      : 4;
        u32         : 1;
        u32 H       : 1;
        u32 S       : 1;
        u32         : 5;
        u32 Rd      : 4;
        u32 Rn      : 4;
        u32 L       : 1;
        u32 W       : 1;
        u32         : 1;
        u32 U       : 1;
        u32 P       : 1;
        u32         : 3;
        u32 Cond    : 4;
    };
};

static_assert(sizeof(HalfwordDataTransferRegOffset::Flags) == sizeof(u32), "HalfwordDataTransferRegOffset::Flags must be 4 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// HalfwordDataTransferImmOffset
///---------------------------------------------------------------------------------------------------------------------------------

struct HalfwordDataTransferImmOffset
{
    static constexpr u32 FORMAT         = 0b0000'0000'0100'0000'0000'0000'1001'0000;
    static constexpr u32 FORMAT_MASK    = 0b0000'1110'0100'0000'0000'0000'1001'0000;
    static constexpr bool IsInstanceOf(u32 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u32 OffsetLo    : 4;
        u32             : 1;
        u32 H           : 1;
        u32 S           : 1;
        u32             : 1;
        u32 OffsetHi    : 4;
        u32 Rd          : 4;
        u32 Rn          : 4;
        u32 L           : 1;
        u32 W           : 1;
        u32             : 1;
        u32 U           : 1;
        u32 P           : 1;
        u32             : 3;
        u32 Cond        : 4;
    };
};

static_assert(sizeof(HalfwordDataTransferImmOffset::Flags) == sizeof(u32), "HalfwordDataTransferImmOffset::Flags must be 4 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// PSRTransferMRS
///---------------------------------------------------------------------------------------------------------------------------------

struct PSRTransferMRS
{
    static constexpr u32 FORMAT         = 0b0000'0001'0000'1111'0000'0000'0000'0000;
    static constexpr u32 FORMAT_MASK    = 0b0000'1111'1011'1111'0000'0000'0000'0000;
    static constexpr bool IsInstanceOf(u32 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u32         : 12;
        u32 Rd      : 4;
        u32         : 6;
        u32 Ps      : 1;
        u32         : 5;
        u32 Cond    : 4;
    };
};

static_assert(sizeof(PSRTransferMRS::Flags) == sizeof(u32), "PSRTransferMRS::Flags must be 4 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// PSRTransferMSR
///---------------------------------------------------------------------------------------------------------------------------------

struct PSRTransferMSR
{
    static constexpr u32 FORMAT         = 0b0000'0001'0010'0000'1111'0000'0000'0000;
    static constexpr u32 FORMAT_MASK    = 0b0000'1101'1011'0000'1111'0000'0000'0000;
    static constexpr bool IsInstanceOf(u32 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u32                 : 16;
        u32 SetControl      : 1;
        u32 SetExtension    : 1;
        u32 SetStatus       : 1;
        u32 SetFlags        : 1;
        u32                 : 2;
        u32 Pd              : 1;
        u32                 : 2;
        u32 I               : 1;
        u32                 : 2;
        u32 Cond            : 4;
    };

    struct RegSrc
    {
        u32 Rm  : 4;
        u32     : 28;
    };

    struct ImmSrc
    {
        u32 Imm     : 8;
        u32 Rotate  : 4;
        u32         : 20;
    };
};

static_assert(sizeof(PSRTransferMSR::Flags) == sizeof(u32), "PSRTransferMSR::Flags must be 4 bytes");
static_assert(sizeof(PSRTransferMSR::RegSrc) == sizeof(u32), "PSRTransferMSR::RegSrc must be 4 bytes");
static_assert(sizeof(PSRTransferMSR::ImmSrc) == sizeof(u32), "PSRTransferMSR::ImmSrc must be 4 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// DataProcessing
///---------------------------------------------------------------------------------------------------------------------------------

struct DataProcessing
{
    static constexpr u32 FORMAT         = 0b0000'0000'0000'0000'0000'0000'0000'0000;
    static constexpr u32 FORMAT_MASK    = 0b0000'1100'0000'0000'0000'0000'0000'0000;
    static constexpr bool IsInstanceOf(u32 instruction) { return (instruction & FORMAT_MASK) == FORMAT; }

    struct Flags
    {
        u32             : 4;
        u32 RegShift    : 1;
        u32             : 7;
        u32 Rd          : 4;
        u32 Rn          : 4;
        u32 S           : 1;
        u32 OpCode      : 4;
        u32 I           : 1;
        u32             : 2;
        u32 Cond        : 4;
    };

    struct RotatedImmSrc
    {
        u32 Imm     : 8;
        u32 Rotate  : 4;
        u32         : 20;
    };

    struct RegShiftedRegSrc
    {
        u32 Rm      : 4;
        u32         : 1;
        u32 ShiftOp : 2;
        u32         : 1;
        u32 Rs      : 4;
        u32         : 20;
    };

    struct ImmShiftedRegSrc
    {
        u32 Rm      : 4;
        u32         : 1;
        u32 ShiftOp : 2;
        u32 Imm     : 5;
        u32         : 20;
    };
};

static_assert(sizeof(DataProcessing::Flags) == sizeof(u32), "DataProcessing::Flags must be 4 bytes");
static_assert(sizeof(DataProcessing::RotatedImmSrc) == sizeof(u32), "DataProcessing::RotatedImmSrc must be 4 bytes");
static_assert(sizeof(DataProcessing::RegShiftedRegSrc) == sizeof(u32), "DataProcessing::RegShiftedRegSrc must be 4 bytes");
static_assert(sizeof(DataProcessing::ImmShiftedRegSrc) == sizeof(u32), "DataProcessing::ImmShiftedRegSrc must be 4 bytes");
}  // namespace cpu::arm
