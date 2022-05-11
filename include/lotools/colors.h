#pragma once
#include <string>
#include <string_view>

namespace lot {

constexpr const char* begin_green = "\033[32m";
constexpr const char* begin_yellow = "\033[33m";
constexpr const char* begin_red = "\033[31m";
constexpr const char* begin_blue = "\033[1;34m";
constexpr const char* color_reset = "\033[0m";

inline std::string green(std::string_view raw_text)
{
    return begin_green + std::string(raw_text) + color_reset;
}

inline std::string yellow(std::string_view raw_text)
{
    return begin_yellow + std::string(raw_text) + color_reset;
}

inline std::string red(std::string_view raw_text)
{
    return begin_red + std::string(raw_text) + color_reset;
}

inline std::string blue(std::string_view raw_text)
{
    return begin_blue + std::string(raw_text) + color_reset;
}

} // namespace lot
