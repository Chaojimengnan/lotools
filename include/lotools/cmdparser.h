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

#ifndef LOT_BEGIN_SIGN
#    define LOT_BEGIN_SIGN '[' // NOLINT(cppcoreguidelines-macro-usage)
#endif

#ifndef LOT_END_SIGN
#    define LOT_END_SIGN ']' // NOLINT(cppcoreguidelines-macro-usage)
#endif

class cmdparser;

struct basic_command // NOLINT(cppcoreguidelines-special-member-functions)
{
    [[nodiscard]] virtual const char* name() const noexcept = 0;
    [[nodiscard]] virtual std::any info(const std::any* info = nullptr) const noexcept
    {
        if (info == nullptr)
            return {};

        return *info;
    }
    virtual void perform(const cmdparser& args) const = 0;
    virtual ~basic_command() = default;
};

struct lambda_command : basic_command
{
    lambda_command(std::string name, std::function<void(const cmdparser&)> handler, const std::function<std::any(const std::any*)>& info_handler = {}) : name_(std::move(name)), handler_(std::move(handler))
    {
        if (info_handler)
            info_handler_ = info_handler;
    }

    [[nodiscard]] const char* name() const noexcept override
    {
        return name_.c_str();
    }

    [[nodiscard]] std::any info(const std::any* info) const noexcept override
    {
        if (info_handler_)
            return info_handler_(info);

        return basic_command::info(info);
    }

    void perform(const cmdparser& args) const override
    {
        handler_(args);
    }

private:
    std::string name_;
    std::function<std::any(const std::any*)> info_handler_;
    std::function<void(const cmdparser& args)> handler_;
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
    cmdparser(int argc, char* argv[]) : argc_(argc)
    {
        for (int i = 1; i < argc; ++i) {
            raw_ += argv[i];
            raw_ += " ";
        }
    }

    void parse()
    {
        lo_assert(!is_parsed_);
        is_parsed_ = true;

        if (argc_ == 0)
            throw args_parse_error("Parameter parsing error: Requires a parameter to specify the command");

        std::string_view raw_view { raw_ };

        auto split_space_item = [&] {
            auto split_pos = raw_view.find(' ');
            auto item = raw_view.substr(0, split_pos);
            raw_view = raw_view.substr(split_pos + 1);
            return item;
        };

        command_name_ = split_space_item();

        while (!raw_view.empty())
        {
            // current_item is an option
            if (raw_view.size() >= 3 && raw_view.substr(0, 2) == "--")
            {
                option_list_.push_back(split_space_item());
                continue;
            }

            // current_item is key-value pair
            auto single_item = raw_view.substr(0, raw_view.find(' '));
            if (auto equal_pos = single_item.find('='),
                second_quote_pos = raw_view.find(LOT_END_SIGN, equal_pos);
                raw_view.size() >= 3 && equal_pos != std::string_view::npos
                && raw_view.at(equal_pos + 1) == LOT_BEGIN_SIGN
                && second_quote_pos != std::string_view::npos)
            {
                std::string_view key = raw_view.substr(0, equal_pos);
                raw_view = raw_view.substr(equal_pos + 2);
                second_quote_pos = raw_view.find(LOT_END_SIGN);
                std::string_view value = raw_view.substr(0, second_quote_pos);
                value_pair_list_.emplace_back(key, value);
                raw_view = raw_view.substr(second_quote_pos + 2);
                continue;
            }

            // current_item is pure value
            single_item = raw_view.substr(0, raw_view.find(' '));
            if (auto first_quote_pos = single_item.find(LOT_BEGIN_SIGN),
                second_quote_pos = raw_view.find(LOT_END_SIGN, first_quote_pos + 1);
                raw_view.size() >= 2 && first_quote_pos != std::string_view::npos
                && second_quote_pos != std::string_view::npos)
            {
                raw_view = raw_view.substr(first_quote_pos + 1);
                second_quote_pos = raw_view.find(LOT_END_SIGN);
                std::string_view value = raw_view.substr(0, second_quote_pos);
                value_list_.emplace_back(value);
                raw_view = raw_view.substr(second_quote_pos + 2);
                continue;
            }

            throw args_parse_error("Parameter parsing error:  \"" + std::string(raw_view) + "\", which isn't one of option(--option), key-value pair(key=" + LOT_BEGIN_SIGN + "value" + LOT_END_SIGN + "), pure value(" + LOT_BEGIN_SIGN + "value" + LOT_END_SIGN + ")");
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

    void add(std::unique_ptr<basic_command> command)
    {
        lo_assert(!is_parsed_);
        command_map_[command->name()] = std::move(command);
    }

    void add(const std::string& name, const std::function<void(const cmdparser& args)>& handler, const std::function<std::any(const std::any*)>& info_handler = {})
    {
        lo_assert(!is_parsed_);
        command_map_[name] = std::make_unique<lambda_command>(name, handler, info_handler);
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

    [[nodiscard]] const std::string& raw() const noexcept
    {
        lo_assert(is_parsed_);
        return raw_;
    }

    [[nodiscard]] const std::string_view& get_command_name() const noexcept
    {
        lo_assert(is_parsed_);
        return command_name_;
    }

    // Option is start with "--", e.g. "--help"
    [[nodiscard]] const std::vector<std::string_view>& get_option_list() const noexcept
    {
        lo_assert(is_parsed_);
        return option_list_;
    }

    // Item pair likes "key=<value>", e.g. "var1=[123]"
    [[nodiscard]] const std::vector<std::pair<std::string_view, std::string_view>>& get_value_pair_list() const noexcept
    {
        lo_assert(is_parsed_);
        return value_pair_list_;
    }

    // Value likes "<value>", e.g. "[123]"
    [[nodiscard]] const std::vector<std::string_view>& get_value_list() const noexcept
    {
        lo_assert(is_parsed_);
        return value_list_;
    }

private:
    int argc_ = 0;
    bool is_vaild_ = false;
    bool is_parsed_ = false;
    std::string raw_;
    std::string_view command_name_;
    std::vector<std::string_view> option_list_;
    std::vector<std::string_view> value_list_;
    std::vector<std::pair<std::string_view, std::string_view>> value_pair_list_;
    std::unordered_map<std::string, std::unique_ptr<basic_command>> command_map_;
};
} // namespace lot