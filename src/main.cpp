#include "lotools/cmdparser.h"

#include <iostream>

struct message
{
    std::string_view usage;
    std::string_view help_tip;
};

struct cmd_help : lot::basic_command
{
    [[nodiscard]] const char* name() const noexcept override
    {
        return "help";
    }

    void perform(const lot::cmdparser& /*args*/) const override
    {
        std::cout << "hehe\n";
    }
};

int main(int argc, char* argv[])
{
    try {
        lot::cmdparser parser(argc, argv);
        parser.add(std::make_unique<cmd_help>());
        parser.add(
            "dada",
            [](const lot::cmdparser&) { std::cout << "my dada\n"; },
            [](const std::any*) { return message { "tc dada", "一个简单的示例" }; });

        auto info = std::any_cast<message>(parser.get_command_map().at("dada")->info());
        std::cout << info.usage << "\n";
        std::cout << info.help_tip << "\n";

        parser.parse();
        parser.exec();

    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
    }
}