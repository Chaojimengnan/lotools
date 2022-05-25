#pragma once
#include <string>
#include <string_view>

namespace lot {

constexpr const char* begin_green = "\033[32m";
constexpr const char* begin_yellow = "\033[33m";
constexpr const char* begin_red = "\033[31m";
constexpr const char* begin_blue = "\033[1;34m";
constexpr const char* color_reset = "\033[0m";

class colors
{
public:
    [[nodiscard]] static bool get_color_switch()
    {
        return color_switch();
    }

    static void set_color_switch(bool is_open)
    {
        color_switch() = is_open;
    }

    static std::string green(std::string_view raw_text)
    {
        if (color_switch())
            return begin_green + std::string(raw_text) + color_reset;
        return std::string(raw_text);
    }

    static std::string yellow(std::string_view raw_text)
    {
        if (color_switch())
            return begin_yellow + std::string(raw_text) + color_reset;
        return std::string(raw_text);
    }

    static std::string red(std::string_view raw_text)
    {
        if (color_switch())
            return begin_red + std::string(raw_text) + color_reset;
        return std::string(raw_text);
    }

    static std::string blue(std::string_view raw_text)
    {
        if (color_switch())
            return begin_blue + std::string(raw_text) + color_reset;
        return std::string(raw_text);
    }

    static std::string begin_greenf()
    {
        if (color_switch())
            return begin_green;
        return {};
    }

    static std::string begin_yellowf()
    {
        if (color_switch())
            return begin_yellow;
        return {};
    }

    static std::string begin_redf()
    {
        if (color_switch())
            return begin_red;
        return {};
    }

    static std::string begin_bluef()
    {
        if (color_switch())
            return begin_blue;
        return {};
    }

    static std::string color_resetf()
    {
        if (color_switch())
            return color_reset;
        return {};
    }

private:
    static bool& color_switch()
    {
        static bool color_switch_ = true;
        return color_switch_;
    }
};

} // namespace lot
