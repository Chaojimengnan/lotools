#pragma once
#include <cassert>

namespace lot {

#if defined(NDEBUG)
#    define lo_assert(expression) // NOLINT(cppcoreguidelines-macro-usage)
#else
#    define lo_assert(expression) assert(expression) // NOLINT(cppcoreguidelines-macro-usage)
#endif

} // namespace lot