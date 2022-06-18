#pragma once

#include "base.h"
#include <any>
#include <array>
#include <cstddef>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <utility>

// TODO : add assert

namespace lot {

namespace detail {

    template <std::uint32_t width, std::uint32_t height, bool is_add_addition>
    struct add_addition_data
    {
        [[nodiscard]] const auto& get_addition_data_map() const noexcept
        {
            return addition_data_;
        }

        [[nodiscard]] auto& get_addition_data_map() noexcept
        {
            return addition_data_;
        }

        [[nodiscard]] bool has_addition_data(std::uint32_t pos_x, std::uint32_t pos_y) const noexcept
        {
            lo_assert(pos_x >= 0 && pos_x < width && pos_y >= 0 && pos_y < height);
            return (addition_data_.find(get_key_from_pos(pos_x, pos_y)) != addition_data_.end());
        }

        [[nodiscard]] std::any& get_addition_data(std::uint32_t pos_x, std::uint32_t pos_y)
        {
            lo_assert(pos_x >= 0 && pos_x < width && pos_y >= 0 && pos_y < height);
            auto iter = addition_data_.find(get_key_from_pos(pos_x, pos_y));

            if (iter != addition_data_.end())
                return iter->second;

            throw std::runtime_error("ascii_screen get_addition_data fails : no addition_data in " + std::to_string(pos_x) + " ," + std::to_string(pos_y));
        }

        static inline std::uint64_t get_key_from_pos(std::uint32_t pos_x, std::uint32_t pos_y) noexcept
        {
            lo_assert(pos_x >= 0 && pos_x < width && pos_y >= 0 && pos_y < height);
            return (std::uint64_t(pos_y) << 32) + pos_x;
        }

        static inline std::pair<std::uint32_t, std::uint32_t> get_pos_from_key(std::uint64_t key) noexcept
        {
            return std::pair { static_cast<std::uint32_t>((key << 32) >> 32), static_cast<std::uint32_t>(key >> 32) };
        }

        [[nodiscard]] auto begin() const noexcept
        {
            return addition_data_.begin();
        }

        [[nodiscard]] auto cbegin() const noexcept
        {
            return begin();
        }

        [[nodiscard]] auto begin() noexcept
        {
            return addition_data_.begin();
        }

        [[nodiscard]] auto end() const noexcept
        {
            return addition_data_.end();
        }

        [[nodiscard]] auto cend() const noexcept
        {
            return end();
        }

        [[nodiscard]] auto end() noexcept
        {
            return addition_data_.end();
        }

    private:
        std::unordered_map<std::uint64_t, std::any> addition_data_;
    };

    template <std::uint32_t width, std::uint32_t height>
    struct add_addition_data<width, height, false>
    {
    };

} // namespace detail

template <std::uint32_t width, std::uint32_t height, bool is_add_addition = false>
class ascii_screen : public detail::add_addition_data<width, height, is_add_addition>
{
public:
    static constexpr char empty_char = ' ';

    ascii_screen() : screen_char_ptr_(std::make_unique<std::array<std::array<char, width>, height>>()) { clear(); }

    [[nodiscard]] constexpr char* data() noexcept
    {
        return reinterpret_cast<char*>(screen_char_ptr_->data()); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    }

    [[nodiscard]] constexpr const char* data() const noexcept
    {
        return screen_char_ptr_->data();
    }

    [[nodiscard]] constexpr auto size() const noexcept
    {
        return width * height;
    }

    [[nodiscard]] constexpr auto& container() noexcept
    {
        return *screen_char_ptr_;
    }

    [[nodiscard]] constexpr const auto& container() const noexcept
    {
        return *screen_char_ptr_;
    }

    ascii_screen& clear()
    {
        if constexpr (is_add_addition)
            for (std::uint32_t pos_y = 0; pos_y < height; pos_y++)
                for (std::uint32_t pos_x = 0; pos_x < width; pos_x++)
                    clear_addition_data(pos_x, pos_y);

        return set(empty_char);
    }

    ascii_screen& clear(std::uint32_t pos_x, std::uint32_t pos_y)
    {
        lo_assert(pos_x >= 0 && pos_x < width && pos_y >= 0 && pos_y < height);
        if constexpr (is_add_addition)
            clear_addition_data(pos_x, pos_y);

        return set(pos_x, pos_y, empty_char);
    }

    ascii_screen& clear_addition_data(std::uint32_t pos_x, std::uint32_t pos_y) requires(is_add_addition)
    {
        lo_assert(pos_x >= 0 && pos_x < width && pos_y >= 0 && pos_y < height);
        this->get_addition_data_map().erase(this->get_key_from_pos(pos_x, pos_y));
        return *this;
    }

    ascii_screen& clear_row(std::uint32_t row, std::uint32_t start = 0, std::uint32_t end = width)
    {
        lo_assert(row >= 0 && row < height);
        lo_assert(start >= 0 && start <= width && start <= end && end <= width);
        if constexpr (is_add_addition)
            for (std::uint32_t pos_x = start; pos_x < end; pos_x++)
                clear_addition_data(pos_x, row);

        return set_row(row, empty_char, start, end);
    }

    ascii_screen& clear_columu(std::uint32_t columu, std::uint32_t start = 0, std::uint32_t end = height)
    {
        lo_assert(columu >= 0 && columu < width);
        lo_assert(start >= 0 && start <= height && start <= end && end <= height);
        if constexpr (is_add_addition)
            for (std::uint32_t pos_y = start; pos_y < end; pos_y++)
                clear_addition_data(columu, pos_y);

        return set_columu(columu, empty_char, start, end);
    }

    ascii_screen& set(std::uint32_t pos_x, std::uint32_t pos_y, char new_character)
    {
        lo_assert(pos_x >= 0 && pos_x < width && pos_y >= 0 && pos_y < height);
        screen_char_ptr_->at(pos_y).at(pos_x) = new_character;
        return *this;
    }

    ascii_screen& set_addition_data(std::uint32_t pos_x, std::uint32_t pos_y, const std::any& addition_data) requires(is_add_addition)
    {
        lo_assert(pos_x >= 0 && pos_x < width && pos_y >= 0 && pos_y < height);
        this->get_addition_data_map()[this->get_key_from_pos(pos_x, pos_y)] = addition_data;
        return *this;
    }

    ascii_screen& set(std::uint32_t pos_x, std::uint32_t pos_y, char new_character, const std::any& addition_data) requires(is_add_addition)
    {
        lo_assert(pos_x >= 0 && pos_x < width && pos_y >= 0 && pos_y < height);
        set(pos_x, pos_y, new_character);
        set_addition_data(pos_x, pos_y, addition_data);
        return *this;
    }

    ascii_screen& set(char new_character)
    {
        std::memset(data(), new_character, size());
        return *this;
    }

    ascii_screen& set_addition_data(const std::any& addition_data) requires(is_add_addition)
    {
        for (std::uint32_t pos_y = 0; pos_y < height; pos_y++)
            for (std::uint32_t pos_x = 0; pos_x < width; pos_x++)
                set_addition_data(pos_x, pos_y, addition_data);
        return *this;
    }

    ascii_screen& set(char new_character, const std::any& addition_data) requires(is_add_addition)
    {
        set(new_character);
        set_addition_data(addition_data);
        return *this;
    }

    ascii_screen& set_row(std::uint32_t row, char new_character, std::uint32_t start = 0, std::uint32_t end = width) // NOLINT(bugprone-easily-swappable-parameters)
    {
        lo_assert(row >= 0 && row < height);
        lo_assert(start >= 0 && start <= width && start <= end && end <= width);
        auto&& row_array = container().at(row);
        std::memset(row_array.data() + start, new_character, end);
        return *this;
    }

    ascii_screen& set_addition_data_row(std::uint32_t row, const std::any& addition_data, std::uint32_t start = 0, std::uint32_t end = width) requires(is_add_addition)
    {
        lo_assert(row >= 0 && row < height);
        lo_assert(start >= 0 && start <= width && start <= end && end <= width);
        for (std::uint32_t pos_x = start; pos_x < end; pos_x++)
            set_addition_data(pos_x, row, addition_data);
        return *this;
    }

    ascii_screen& set_row(std::uint32_t row, char new_character, const std::any& addition_data, std::uint32_t start = 0, std::uint32_t end = width) requires(is_add_addition)
    {
        lo_assert(row >= 0 && row < height);
        lo_assert(start >= 0 && start <= width && start <= end && end <= width);
        set_row(row, new_character, start, end);
        set_addition_data_row(row, addition_data, start, end);
        return *this;
    }

    ascii_screen& set_columu(std::uint32_t columu, char new_character, std::uint32_t start = 0, std::uint32_t end = height) // NOLINT(bugprone-easily-swappable-parameters)
    {
        lo_assert(columu >= 0 && columu < width);
        lo_assert(start >= 0 && start <= height && start <= end && end <= height);
        auto* columu_ptr = container().at(start).data() + columu;
        for (std::uint32_t index = 0; index < end; ++index) {
            *columu_ptr = new_character;
            columu_ptr += width;
        }
        return *this;
    }

    ascii_screen& set_addition_data_columu(std::uint32_t columu, const std::any& addition_data, std::uint32_t start = 0, std::uint32_t end = height) requires(is_add_addition)
    {
        lo_assert(columu >= 0 && columu < width);
        lo_assert(start >= 0 && start <= height && start <= end && end <= height);
        for (std::uint32_t pos_y = start; pos_y < end; pos_y++)
            set_addition_data(columu, pos_y, addition_data);
        return *this;
    }

    ascii_screen& set_columu(std::uint32_t columu, char new_character, const std::any& addition_data, std::uint32_t start = 0, std::uint32_t end = width) requires(is_add_addition)
    {
        lo_assert(columu >= 0 && columu < width);
        lo_assert(start >= 0 && start <= height && start <= end && end <= height);
        set_columu(columu, new_character, start, end);
        set_addition_data_columu(columu, addition_data, start, end);
        return *this;
    }

    char get(std::uint32_t pos_x, std::uint32_t pos_y)
    {
        lo_assert(pos_x >= 0 && pos_x < width && pos_y >= 0 && pos_y < height);
        return screen_char_ptr_->at(pos_y).at(pos_x);
    }

    ascii_screen& show(std::ostream& out)
    {
        auto&& screen = container();
        for (auto&& row : screen) {
            out.write(row.data(), row.size());
            out << "\n";
        }
        return *this;
    }

private:
    std::unique_ptr<std::array<std::array<char, width>, height>> screen_char_ptr_;
};

} // namespace lot