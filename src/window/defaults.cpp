#include "defaults.h"

DefaultParams loadDefaults() {
    DefaultParams params;
    try {
        auto data = toml::parse("data/defaults.toml");
        params.is_inline = toml::find_or<bool>(data, "is_inline", false);
        params.font_size = toml::find_or<int>(data, "font_size", 50);
        params.font_family = toml::find_or<std::string>(data, "font_family", "XITS");
        params.font_family_idx = toml::find_or<size_t>(data, "font_family_idx", 0);
        auto vec = toml::find_or<std::vector<float>>(data, "text_color", { 0.f, 0.f, 0.f, 1.f });
        params.text_color = ImVec4(vec[0], vec[1], vec[2], vec[3]);
        vec = toml::find_or<std::vector<float>>(data, "background_color", { 1.f, 1.f, 1.f, 1.f });
        params.background_color = ImVec4(vec[0], vec[1], vec[2], vec[3]);
    }
    catch (const std::exception& e) {
        std::cerr << "Error while loading defaults.toml: " << e.what() << std::endl;
    }
    return params;
}

void saveDefaults(const DefaultParams& params) {
    toml::value data;
    data["is_inline"] = params.is_inline;
    data["font_size"] = params.font_size;
    data["font_family"] = params.font_family;
    data["font_family_idx"] = params.font_family_idx;
    data["text_color"] = std::vector<float>{ params.text_color.x, params.text_color.y, params.text_color.z, params.text_color.w };
    data["background_color"] = std::vector<float>{ params.background_color.x, params.background_color.y, params.background_color.z, params.background_color.w };
    std::ofstream file("data/defaults.toml");
    file << data;
    file.close();
}