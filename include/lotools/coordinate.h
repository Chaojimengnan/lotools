#pragma once
#include "base.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>

namespace lot {

/**
 * @brief A simple coordinate template that can represent coordinates in any dimension, 
 * internally using std::array to store the coordinates
 * 
 * @tparam T Data type of coordinate
 * @tparam dimension The number of dimensions
 */
template <typename T, std::size_t dimension>
struct coordinate
{
    static_assert(std::is_arithmetic_v<T>, "T must be an arithmetic type!");

    using DataType = std::array<T, dimension>;

    constexpr coordinate() = default;
    constexpr coordinate(std::initializer_list<T> val) : data()
    {
        lo_assert(val.size() == dimension);
        std::copy(val.begin(), val.end(), data.begin());
    }

    constexpr coordinate operator-() const noexcept
    {
        coordinate temp {};
        std::transform(data.cbegin(), data.cend(), temp.data.begin(), [](const T& val) { return -val; });
        return temp;
    }

    constexpr coordinate& operator+=(const coordinate& rhs) noexcept
    {
        std::transform(
            data.cbegin(), data.cend(), rhs.data.cbegin(), data.begin(), [](const T& rhs, const T& lhs) constexpr { return rhs + lhs; });
        return *this;
    }

    constexpr coordinate& operator-=(const coordinate& rhs) noexcept
    {
        std::transform(
            data.cbegin(), data.cend(), rhs.data.cbegin(), data.begin(), [](const T& rhs, const T& lhs) constexpr { return rhs - lhs; });
        return *this;
    }

    [[nodiscard]] constexpr double distance() const noexcept
    {
        return std::sqrt(distance_rough());
    }

    [[nodiscard]] constexpr std::size_t distance_rough() const noexcept
    {
        DataType temp;
        std::transform(
            data.cbegin(), data.cend(), temp.begin(), [](const T& val) constexpr { return val * val; });
        return std::accumulate(temp.cbegin(), temp.cend(), 0);
    }

    [[nodiscard]] constexpr coordinate abs() const noexcept
    {
        DataType temp;
        std::transform(
            data.cbegin(), data.cend(), temp.begin(), [](const T& val) constexpr { return std::abs(val); });
        return temp;
    }

    friend constexpr coordinate<T, dimension> operator+(const coordinate<T, dimension>& lhs, const coordinate<T, dimension>& rhs) noexcept
    {
        coordinate<T, dimension> temp {};
        std::transform(
            lhs.data.cbegin(), lhs.data.cend(), rhs.data.cbegin(), temp.data.begin(), [](const T& lhs, const T& rhs) constexpr { return lhs + rhs; });
        return temp;
    }

    friend constexpr coordinate<T, dimension> operator-(const coordinate<T, dimension>& lhs, const coordinate<T, dimension>& rhs) noexcept
    {
        coordinate<T, dimension> temp {};
        std::transform(
            lhs.data.cbegin(), lhs.data.cend(), rhs.data.cbeing(), temp.data.begin(), [](const T& lhs, const T& rhs) constexpr { return lhs - rhs; });
        return temp;
    }

    friend constexpr bool operator==(const coordinate<T, dimension>& lhs, const coordinate<T, dimension>& rhs) noexcept
    {
        std::array<bool, dimension> temp {};
        std::transform(
            lhs.data.cbegin(), lhs.data.cend(), rhs.data.cbegin(), temp.begin(), [](const T& lhs, const T& rhs) constexpr { return lhs == rhs; });
        return std::all_of(
            temp.cbegin(), temp.cend(), [](bool val) constexpr { return val; });
    }

    friend constexpr bool operator!=(const coordinate<T, dimension>& lhs, const coordinate<T, dimension>& rhs) noexcept
    {
        return !(lhs == rhs);
    }

    friend std::ostream& operator<<(std::ostream& lhs, const coordinate<T, dimension>& rhs)
    {
        return lhs << rhs.to_string();
    }

    friend std::istream& operator>>(std::istream& in, coordinate<T, dimension>& rhs)
    {
        char c = '\0';
        bool succesd = true;
        while (true)
        {
            if (!(in.get(c) && c == '('))
            {
                succesd = false;
                break;
            }

            for (size_t i = 0; i < rhs.data.size(); i++)
            {
                if (!(in >> rhs.data.at(i) && in.get(c)))
                {
                    succesd = false;
                    break;
                }

                if (c == ')')
                    break;

                if (c != ',')
                {
                    succesd = false;
                    break;
                }
            }

            break;
        }

        if (!succesd)
            throw std::runtime_error("From string to data fails!");

        return in;
    }

    [[nodiscard]] std::string to_string() const
    {
        // return fmt::format("({},{})", x, y);
        std::stringstream o;
        o << "(";
        o << data.at(0);
        for (size_t i = 1; i < data.size(); i++)
        {
            o << ",";
            o << data.at(i);
        }

        o << ")";
        return o.str();
    }

    static coordinate<T, dimension> from_string(std::string_view str)
    {
        std::istringstream in(str.data());
        coordinate<T, dimension> temp {};
        in >> temp;
        return temp;
    }

    DataType data;
};

template<typename T>
using point = coordinate<T, 2>;

template<typename T>
using tripoint = coordinate<T, 3>;

} // namespace lot
