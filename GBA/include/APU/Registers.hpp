#pragma once

#include <GBA/include/Utilities/Types.hpp>

namespace audio
{
///---------------------------------------------------------------------------------------------------------------------------------
/// Channel 1 registers
///---------------------------------------------------------------------------------------------------------------------------------

struct SOUND1CNT
{
    // Lo
    u64 step            : 3;
    u64 sweepDirection  : 1;
    u64 sweepPace       : 3;
    u64                 : 9;

    // Hi
    u64 initialLengthTimer  : 6;
    u64 waveDuty            : 2;
    u64 envelopePace        : 3;
    u64 envelopeDirection   : 1;
    u64 initialVolume       : 4;

    // X
    u64 period          : 11;
    u64                 : 3;
    u64 lengthEnable    : 1;
    u64 trigger         : 1;

    // Unused
    u64 : 16;
};

static_assert(sizeof(SOUND1CNT) == 8, "SOUND1CNT must be 8 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// Channel 2 registers
///---------------------------------------------------------------------------------------------------------------------------------

struct SOUND2CNT
{
    // Lo
    u64 initialLengthTimer  : 6;
    u64 waveDuty            : 2;
    u64 envelopePace        : 3;
    u64 envelopeDirection   : 1;
    u64 initialVolume       : 4;

    // Unused
    u64 : 16;

    // Hi
    u64 period          : 11;
    u64                 : 3;
    u64 lengthEnable    : 1;
    u64 trigger         : 1;

    // Unused
    u64 : 16;
};

static_assert(sizeof(SOUND2CNT) == 8, "SOUND2CNT must be 8 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// Channel 4 registers
///---------------------------------------------------------------------------------------------------------------------------------

struct SOUND4CNT
{
    // Lo
    u64 initialLengthTimer  : 6;
    u64                     : 2;
    u64 envelopePace        : 3;
    u64 envelopeDirection   : 1;
    u64 initialVolume       : 4;

    // Unused
    u64 : 16;

    // Hi
    u64 dividingRatio       : 3;
    u64 countWidth          : 1;
    u64 shiftClockFrequency : 4;
    u64                     : 6;
    u64 lengthEnable        : 1;
    u64 trigger             : 1;

    // Unused
    u64 : 16;
};

static_assert(sizeof(SOUND4CNT) == 8, "SOUND4CNT must be 8 bytes");

///---------------------------------------------------------------------------------------------------------------------------------
/// Control registers
///---------------------------------------------------------------------------------------------------------------------------------

struct SOUNDCNT_L
{
    static constexpr size_t INDEX = 0;

    u16 psgRightMasterVolume    : 3;
    u16                         : 1;
    u16 psgLeftMasterVolume     : 3;
    u16                         : 1;
    u16 chan1EnableRight        : 1;
    u16 chan2EnableRight        : 1;
    u16 chan3EnableRight        : 1;
    u16 chan4EnableRight        : 1;
    u16 chan1EnableLeft         : 1;
    u16 chan2EnableLeft         : 1;
    u16 chan3EnableLeft         : 1;
    u16 chan4EnableLeft         : 1;
};

static_assert(sizeof(SOUNDCNT_L) == 2, "SOUNDCNT_L must be 2 bytes");

struct SOUNDCNT_H
{
    static constexpr size_t INDEX = 2;

    u16 psgVolume       : 2;
    u16 dmaVolumeA      : 1;
    u16 dmaVolumeB      : 1;
    u16                 : 4;
    u16 dmaEnableRightA : 1;
    u16 dmaEnableLeftA  : 1;
    u16 dmaTimerSelectA : 1;
    u16 dmaResetA       : 1;
    u16 dmaEnableRightB : 1;
    u16 dmaEnableLeftB  : 1;
    u16 dmaTimerSelectB : 1;
    u16 dmaResetB       : 1;
};

static_assert(sizeof(SOUNDCNT_H) == 2, "SOUNDCNT_H must be 2 bytes");

struct SOUNDCNT_X
{
    static constexpr size_t INDEX = 4;

    u16 chan1On         : 1;
    u16 chan2On         : 1;
    u16 chan3On         : 1;
    u16 chan4On         : 1;
    u16                 : 3;
    u16 masterEnable    : 1;
    u16                 : 8;
};

static_assert(sizeof(SOUNDCNT_X) == 2, "SOUNDCNT_X must be 2 bytes");

struct SOUNDBIAS
{
    static constexpr size_t INDEX = 8;

    u16                 : 1;
    u16 biasLevel       : 9;
    u16                 : 4;
    u16 samplingCycle   : 2;
};

static_assert(sizeof(SOUNDBIAS) == 2, "SOUNDBIAS must be 2 bytes");
}  // namespace audio
