#include "utility.h"
#include <utility>

namespace lot {

/**
 * @brief Handle function that return error code to indicate error.
 *
 * @param reset_func Called after the `handler` when `cond` returns true, can be used to reset errors, such as unix `errno = 0`. (auto*)
 * @param handler Called when `cond` returns true, throw exception is ok. (const char* filename, int line, const char* funcname, auto*)
 * @param cond `handler` and `reset_func` are called when `cond` returns true. (auto*)
 * @param func Target function
 * @param args The parameters to be passed to the target function
 * @return decltype(auto) Return value of the target function
 */
template <typename ResetFunc, typename Handler, typename Condition, typename Func, typename... Args>
decltype(auto) forward_call(const char* filename, int line, const char* funcname, ResetFunc reset_func, Handler handler, Condition cond, Func func, Args&&... args)
{
    if constexpr (!std::is_same_v<void, decltype(func(std::forward<Args>(args)...))>)
    {
        decltype(auto) val = func(std::forward<Args>(args)...);
        if (cond(&val))
        {
            try {
                handler(filename, line, funcname, &val);
            } catch (...) {
                reset_func(&val);
                throw;
            }
            reset_func(&val);
        }
        return val;
    } else {
        func(std::forward<Args>(args)...);
        if (cond(nullptr))
        {
            try {
                handler(filename, line, funcname, nullptr);
            } catch (...) {
                reset_func(nullptr);
                throw;
            }
            reset_func(nullptr);
        }
    }
}

#ifndef lotcall
#    define lotcall(reset_func, handler, cond, func, ...) ::lot::forward_call(::lot::get_file_name(__FILE__), __LINE__, __FUNCTION__, reset_func, handler, cond, func, __VA_ARGS__) // NOLINT(cppcoreguidelines-macro-usage)
#endif

} // namespace lot