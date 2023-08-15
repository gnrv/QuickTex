#include "commands.h"

#include <fstream>
#include <iostream>

namespace Search {
    void CommandSearch::init() {
        {
            std::fstream fs("data/suggestions.toml", std::ios::in);
            auto file = toml::parse("data/suggestions.toml");

            auto cmds = toml::find_or<toml::array>(file, "commands", toml::array());
            for (auto item : cmds) {
                auto arr = toml::find_or<std::vector<size_t>>(item, "indices", std::vector<size_t>());
                std::string str = item["str"].as_string();
                Command cmd{ str, arr, (int)item["frequency"].as_integer() };
                m_cmds_strs.push_back(str);
                m_str_to_command[str] = cmd;
            }
        }
        {
            std::fstream fs("data/simple_commands.toml", std::ios::in);
            auto file = toml::parse("data/simple_commands.toml");

            auto cmds = toml::find_or<toml::array>(file, "commands", toml::array());
            for (auto item : cmds) {
                std::string str = item["str"].as_string();
                Command cmd{ str, {str.size()}, (int)item["frequency"].as_integer() };
                m_cmds_strs.push_back(str);
                m_str_to_command[str] = cmd;
            }
        }
    }

    CommandSearch::CommandSearch() {
        init();
    }

    std::vector<Command> CommandSearch::getBestSuggestions(const std::string& query, int num_suggestions) {
        std::vector<std::string> results_phrases;
        auto result_scores = ScoreQueryAgainstResults({ query }, m_cmds_strs, " ");

        std::vector<Command> results;
        int i = 0;
        for (auto it = result_scores.rbegin();it != result_scores.rend();it++) {
            for (auto result : it->second) {
                if (i > num_suggestions)
                    return results;
                results.push_back(m_str_to_command[result]);
                i++;
            }
        }
        return results;
    }
}