#pragma once

#include <memory>
#include <type_traits>
#include <utility>

namespace lot {

namespace detail {
    template <typename ReturnType>
    struct return_val_checker
    {
        ReturnType return_val;
    };

    // When ReturnType is void, no member 'return_val'
    template <>
    struct return_val_checker<void>
    {
    };
} // namespace detail

/**
 * @brief A RAII-based tool class, which will call the specified initialization
 * function on construction and the specified destructor on destructon.
 * Suitable for some C libraries that need to call initialization and destruction
 *
 * @tparam init_func Initialize function instances, such as function pointers or callable objects
 * @tparam destroy_func Destruction function instances, such as function pointers or callable objects(it should be noexcept for best)
 */
template <auto init_func, auto destroy_func>
struct raii_control : public detail::return_val_checker<decltype(init_func())>
{
    raii_control(const raii_control&) = delete;
    raii_control(raii_control&&) = delete;
    raii_control& operator=(const raii_control&) = delete;
    raii_control& operator=(raii_control&&) = delete;

    using ReturnType = decltype(init_func());

    template <typename... Args>
    raii_control(Args&&... args)
    {
        if constexpr (std::is_same_v<void, ReturnType>)
        {
            init_func(std::move(args)...);
        } else {
            this->return_val = init_func(std::move(args)...);
        }
    }

    ~raii_control() noexcept(noexcept(destroy_func()))
    {
        destroy_func();
    }
};

namespace detail {
    template <typename Del>
    struct add_del_if_not_void
    {
        add_del_if_not_void() = default;
        add_del_if_not_void(Del del, bool is_del) : del(std::move(del)), is_del(is_del) { }
        Del del;            // NOLINT(misc-non-private-member-variables-in-classes)
        bool is_del = true; // NOLINT(misc-non-private-member-variables-in-classes)
    };

    template <>
    struct add_del_if_not_void<void>
    {
    };
} // namespace detail

/**
 * @brief Likes unique_ptr, but manage value not pointer! You can custom your own deleter.
 * @tparam T Value type
 * @tparam Del Deleter Type
 */
template <typename T, typename Del = void>
struct unique_val : private detail::add_del_if_not_void<Del>
{
    using base = detail::add_del_if_not_void<Del>;

    template <typename T2 = T, typename Del2 = Del, std::enable_if_t<std::is_default_constructible_v<T2> && (std::is_same_v<Del2, void> || std::is_default_constructible_v<Del2>), int> = 0>
    constexpr unique_val() {}; // NOLINT(modernize-use-equals-default)

    template <typename Del2 = Del, std::enable_if_t<!std::is_same_v<Del2, void>, int> = 0>
    constexpr unique_val(const T& val, Del2 del) : base(del, true), val(val) { }

    template <typename Del2 = Del, std::enable_if_t<std::is_same_v<Del2, void> || std::is_default_constructible_v<Del2>, int> = 0>
    explicit unique_val(const T& val) : val(val) { }

    unique_val(const unique_val&) = delete;
    unique_val& operator=(const unique_val&) = delete;

    unique_val(unique_val&& right) noexcept : val(right.val), base(std::move(right))
    {
        if constexpr (!std::is_same_v<Del, void>)
            right.is_del = false;
    }

    unique_val& operator=(unique_val&& right) noexcept
    {
        if (this != &right)
        {
            if constexpr (!std::is_same_v<Del, void>)
                if (this->is_del)
                    this->del(val);

            val = right.val;
            if constexpr (!std::is_same_v<Del, void>)
            {
                this->is_del = true;
                this->del = right.del;
                right.is_del = false;
            }
        }
        return *this;
    }

    ~unique_val()
    {
        if constexpr (!std::is_same_v<Del, void>)
            if (this->is_del)
                this->del(val);
    }

    [[nodiscard]] T& get() noexcept
    {
        return val;
    }

    [[nodiscard]] const T& get() const noexcept
    {
        return val;
    }

    template <typename Del2 = Del, std::enable_if_t<!std::is_same_v<Del2, void>, int> = 0>
    void release() noexcept
    {
        this->is_del = false;
    }

private:
    T val;
};

// https://stackoverflow.com/a/51274008/15128365

template <auto fn>
struct ptr_deleter_from
{
    template <typename T>
    constexpr void operator()(T* arg) const
    {
        fn(arg);
    }
};

template <auto fn>
struct val_deleter_from
{
    template <typename T>
    constexpr void operator()(T& arg) const
    {
        fn(arg);
    }
};

template <typename T, auto fn>
using fn_unique_ptr = std::unique_ptr<T, ptr_deleter_from<fn>>;

template <typename T, auto fn>
using fn_unique_val = unique_val<T, val_deleter_from<fn>>;

} // namespace lot