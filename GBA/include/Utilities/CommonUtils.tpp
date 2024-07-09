#pragma once

#include <GBA/include/Utilities/CommonUtils.hpp>
#include <limits>
#include <stdexcept>
#include <type_traits>

template <typename T, u8 index>
T SignExtend(T input)
{
    static_assert(std::is_signed_v<T>, "SignExtend template param must be a signed type");
    static_assert(std::is_integral_v<T>, "SignExtend template param must be an integral type");
    static_assert(index < std::numeric_limits<T>::digits, "Index cannot exceed number of bits in output type");

    constexpr T signMask = 0x01 << index;
    constexpr T inputMask = signMask | (signMask - 1);
    input &= inputMask;
    T output;

    if ((input & signMask) == signMask)
    {
        output = input | ~inputMask;
    }
    else
    {
        output = input;
    }

    return output;
}
