#pragma once
#include <cassert>

namespace lot {

#if defined(NDEBUG)
#define lo_assert(expression)
#else
#define lo_assert(expression) assert(expression)
#endif


} // namespace lot