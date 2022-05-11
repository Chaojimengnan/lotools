#pragma once

#include <cstddef>
#include <limits>
#include <type_traits>

namespace lot {

template <std::size_t exponential, std::size_t base = 10>
struct power
{
    static constexpr std::size_t pow = base * power<exponential - 1, base>::pow;
};

template <std::size_t base>
struct power<0, base>
{
    static constexpr std::size_t pow = 1;
};

template <typename IntegerType>
constexpr std::size_t get_value_digit(IntegerType value = std::numeric_limits<IntegerType>::max())
{
    static_assert(std::is_integral_v<IntegerType>, "IntegerType must be integral type");
    int digit = 0;

    while (value != 0) {
        value /= 10;
        ++digit;
    }
    return digit;
}

} // namespace lot