#pragma once

namespace lot {

// QT, Precisely specifying overloads
// https://doc.qt.io/qt-6/qtglobal.html#qOverload

template <typename... Args>
struct const_overload
{
    // member func，const
    template <typename R, typename T>
    static constexpr auto of(R (T::*ptr)(Args...)) noexcept -> decltype(ptr)
    {
        return ptr;
    }
};

template <typename... Args>
struct nonconst_overload
{
    // member func，non-const
    template <typename R, typename T>
    static constexpr auto of(R (T::*ptr)(Args...) const) noexcept -> decltype(ptr)
    {
        return ptr;
    }
};

template <typename... Args>
struct overload : const_overload<Args...>, nonconst_overload<Args...>
{

    using const_overload<Args...>::of;
    using nonconst_overload<Args...>::of;

    // normal func
    template <typename R>
    static constexpr auto of(R (*ptr)(Args...)) noexcept -> decltype(ptr)
    {
        return ptr;
    }
};

} // namespace lot