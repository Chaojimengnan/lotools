#pragma once

#include <type_traits>

namespace lot {

/**
 * @brief A template meta-tool that uses BoolOper and T to test every type starting with U until BoolOper gets true,
 * at which point the 'type' member is the type of U that succeeded in the test,
 * and if all types are false, then the final 'type' member is void
 *
 * @tparam at_left If true, BoolOper<T, U>, otherwise BoolOper<U, T>
 * @tparam BoolOper A test template which should have a 'value' member to indicate whether the test was successful, e.g. std::is_same
 * @tparam T The type to test
 * @tparam U First type
 * @tparam Rest Other types
 */
template <bool at_left, template <typename, typename> typename BoolOper, typename T, typename U, typename... Rest>
struct any_type_true
{
    using type = std::conditional_t<BoolOper<T, U>::value, U, typename any_type_true<at_left, BoolOper, T, Rest...>::type>;
};

template <bool at_left, template <typename, typename> typename BoolOper, typename T, typename U>
struct any_type_true<at_left, BoolOper, T, U>
{
    using type = std::conditional_t<BoolOper<T, U>::value, U, void>;
};

template <template <typename, typename> typename BoolOper, typename T, typename U, typename... Rest>
struct any_type_true<false, BoolOper, T, U, Rest...>
{
    using type = std::conditional_t<BoolOper<U, T>::value, U, typename any_type_true<false, BoolOper, T, Rest...>::type>;
};

template <template <typename, typename> typename BoolOper, typename T, typename U>
struct any_type_true<false, BoolOper, T, U>
{
    using type = std::conditional_t<BoolOper<U, T>::value, U, void>;
};

template <bool at_left, template <typename, typename> typename BoolOper, typename T, typename U, typename... Rest>
using any_type_true_t = typename any_type_true<at_left, BoolOper, T, U, Rest...>::type;

} // namespace lot