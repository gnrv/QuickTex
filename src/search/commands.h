#pragma once

#include "search.h"

#include <unordered_map>
#include <toml.hpp>

namespace Search {
    struct Command {
        std::string str;
        std::vector<size_t> indices;
        int frequency = 0;
    };

    class CommandSearch {
    private:
        std::vector<Command> m_commands;
        std::vector<std::string> m_cmds_strs;
        std::unordered_map<std::string, Command> m_str_to_command;

        void init();
    public:
        CommandSearch();

        std::vector<Command> getBestSuggestions(const std::string& query, int num_suggestions = 5);
    };
}