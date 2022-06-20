#pragma once

#include <any>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "base.h"

namespace lot {

class cmdparser;

struct basic_command // NOLINT(cppcoreguidelines-special-member-functions)
{
    [[nodiscard]] constexpr virtual const char* name() const noexcept = 0;
    [[nodiscard]] virtual std::any info(const std::any* info = nullptr) const noexcept
    {
        if (info == nullptr)
            return {};

        return *info;
    }
    virtual constexpr void perform(const cmdparser& args) const = 0;
    virtual ~basic_command() = default;
};

template <auto handler, auto info_handler = nullptr>
struct lambda_command : basic_command
{
    lambda_command(std::string name) : name_(std::move(name))
    {
    }

    [[nodiscard]] constexpr const char* name() const noexcept override
    {
        return name_.c_str();
    }

    [[nodiscard]] std::any info(const std::any* info) const noexcept override
    {
        if constexpr (info_handler != nullptr)
            return info_handler(info);

        return basic_command::info(info);
    }

    constexpr void perform(const cmdparser& args) const override
    {
        handler(args);
    }

private:
    std::string name_;
};

class args_parse_error : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

class command_not_found_error : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

class cmdparser
{
public:
    cmdparser(int argc, char* argv[])
    {
        for (int i = 1; i < argc; ++i)
            raw_.emplace_back(argv[i]);
    }

    void parse()
    {
        lo_assert(!is_parsed_);
        is_parsed_ = true;

        if (std::size(raw_) == 0)
            throw args_parse_error("Parameter parsing error: Requires a parameter to specify the command");

        command_name_ = raw_[0];

        for (auto iter = ++raw_.begin(); iter != raw_.end(); ++iter)
        {
            auto&& item = *iter;

            // current_item is an option pair
            if (auto equal_pos = item.find('=');
                equal_pos != std::string_view::npos && item.size() >= 3
                && item.starts_with("--"))
            {
                std::string_view key = item.substr(0, equal_pos);
                std::string_view value = item.substr(equal_pos + 1);
                option_pair_list_.emplace_back(key, value);
                continue;
            }

            // current_item is an option
            if (item.size() >= 3 && item.starts_with("--"))
            {
                option_list_.push_back(item);
                continue;
            }

            // current_item is key-value pair
            if (auto equal_pos = item.find('=');
                equal_pos != std::string_view::npos)
            {
                std::string_view key = item.substr(0, equal_pos);
                std::string_view value = item.substr(equal_pos + 1);
                value_pair_list_.emplace_back(key, value);
                continue;
            }

            // current_item is pure value
            value_list_.emplace_back(item);

            // throw args_parse_error("Parameter parsing error:  \"" + std::string(item) + "\", which isn't one of option(--option), key-value pair(key=value), pure value(value)");
        }

        is_vaild_ = true;
    }

    void exec()
    {
        lo_assert(is_parsed_);

        auto cmd_name = std::string(command_name_);
        auto iter = command_map_.find(cmd_name);

        if (iter == command_map_.cend())
            throw command_not_found_error("Unknown command : " + cmd_name);

        iter->second->perform(*this);
    }

    cmdparser& add(std::unique_ptr<basic_command> command)
    {
        lo_assert(!is_parsed_);
        command_map_[command->name()] = std::move(command);
        return *this;
    }

    template <auto handler, auto info_handler = nullptr>
    cmdparser& add(const std::string& name)
    {
        lo_assert(!is_parsed_);
        command_map_[name] = std::make_unique<lambda_command<handler, info_handler>>(name);
        return *this;
    }

    [[nodiscard]] const std::unordered_map<std::string, std::unique_ptr<basic_command>>& get_command_map() const noexcept
    {
        return command_map_;
    }

    [[nodiscard]] bool is_vaild() const noexcept
    {
        lo_assert(is_parsed_);
        return is_vaild_;
    }

    [[nodiscard]] const std::vector<std::string_view>& raw() const noexcept
    {
        lo_assert(is_parsed_);
        return raw_;
    }

    [[nodiscard]] const std::string_view& get_command_name() const noexcept
    {
        lo_assert(is_parsed_);
        return command_name_;
    }

    // Option pair likes "--option=value", e.g. "--password=123"
    [[nodiscard]] const std::vector<std::pair<std::string_view, std::string_view>>& get_option_pair_list() const noexcept
    {
        lo_assert(is_parsed_);
        return option_pair_list_;
    }

    // Option is start with "--", e.g. "--help"
    [[nodiscard]] const std::vector<std::string_view>& get_option_list() const noexcept
    {
        lo_assert(is_parsed_);
        return option_list_;
    }

    // Item pair likes "key=value", e.g. "var1=123"
    [[nodiscard]] const std::vector<std::pair<std::string_view, std::string_view>>& get_value_pair_list() const noexcept
    {
        lo_assert(is_parsed_);
        return value_pair_list_;
    }

    // Value likes "value", e.g. "123"
    [[nodiscard]] const std::vector<std::string_view>& get_value_list() const noexcept
    {
        lo_assert(is_parsed_);
        return value_list_;
    }

private:
    bool is_vaild_ = false;
    bool is_parsed_ = false;
    std::string_view command_name_;
    std::vector<std::string_view> option_list_;
    std::vector<std::string_view> value_list_;
    std::vector<std::string_view> raw_;
    std::vector<std::pair<std::string_view, std::string_view>> value_pair_list_;
    std::vector<std::pair<std::string_view, std::string_view>> option_pair_list_;
    std::unordered_map<std::string, std::unique_ptr<basic_command>> command_map_;
};
} // namespace lot