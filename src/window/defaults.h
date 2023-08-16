#pragma once

#include <toml.hpp>
#include <tempo.h>
#include "core/colors.h"

struct DefaultParams {
    bool is_inline = false;
    int font_size = 50;
    std::string font_family = "XITS";
    size_t font_family_idx = 0;
    ImVec4 text_color = ImGui::ColorConvertU32ToFloat4(Colors::black);
    ImVec4 background_color = ImGui::ColorConvertU32ToFloat4(Colors::white);
};

DefaultParams loadDefaults();

void saveDefaults(const DefaultParams& params);