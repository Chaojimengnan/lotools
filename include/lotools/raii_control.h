#pragma once

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
 * @tparam InitFunc Type of initialization function. You can use `decltype(your_init_function)`
 * @tparam DestroyFunc Type of destructon function. You can use `decltype(your_destory_function)`
 * @tparam init_func Initialize function instances, such as function pointers or callable objects
 * @tparam destroy_func Destruction function instances, such as function pointers or callable objects
 */
template <typename InitFunc, typename DestroyFunc, InitFunc init_func, DestroyFunc destroy_func>
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
        add_del_if_not_void(Del del, bool is_del) : del(std::move(del)), is_del(is_del) { }
        Del del;
        bool is_del = true;
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
    constexpr unique_val() {};

    template <typename Del2 = Del, std::enable_if_t<!std::is_same_v<Del2, void>, int> = 0>
    constexpr unique_val(const T& val, Del2 del) : base(del, true), val(val) { }

    template <typename Del2 = Del, std::enable_if_t<std::is_same_v<Del2, void> || std::is_default_constructible_v<Del2>, int> = 0>
    explicit unique_val(const T& val) : val(val) { }

    unique_val(const unique_val&) = delete;
    unique_val& operator=(const unique_val&) = delete;

    unique_val(unique_val&& u) noexcept : val(u.val), base(std::move(u))
    {
        if constexpr (!std::is_same_v<Del, void>)
            u.is_del = false;
    }

    unique_val& operator=(unique_val&& u) noexcept
    {
        if (this != &u)
        {
            if constexpr (!std::is_same_v<Del, void>)
                if (this->is_del)
                    this->del(val);

            val = u.val;
            if constexpr (!std::is_same_v<Del, void>)
            {
                this->is_del = true;
                this->del = u.del;
                u.is_del = false;
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

} // namespace lot