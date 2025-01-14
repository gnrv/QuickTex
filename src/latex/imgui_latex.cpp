#include "imgui_latex.h"

#include "imgui_internal.h"
#include "imgui_scale.h"

#include "latex.h"

#include <iostream>
#include <memory>

struct ImLatex {
    std::string src;
    std::unique_ptr<Latex::LatexImage> image;
    int wrap_pos_x{ 0 };
    bool animate{ false };
    float scale{ 1.f };
};

namespace ImGui {
    ImPool<ImLatex> g_Latexes;

    void InitLatex() {
        std::string err = Latex::init();
        if (!err.empty()) {
            std::cerr << "Failed to initialize MicroTeX: " << err.c_str();
        }
    }

    void Latex(const char* src, ImGuiLatexFlags flags) {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return;

        const ImGuiID id = ImGui::GetID(src);
        ImLatex *latex = g_Latexes.GetOrAddByKey(id);
        const float wrap_pos_x = window->DC.TextWrapPos;
        // The src is the bit before the ###
        const char* src_end = src + strlen(src);
        if (const char* p = strstr(src, "###"))
            src_end = p;
        std::string_view src_view(src, src_end - src);
        float scale = GetScale();
        if (latex->src != src_view || latex->wrap_pos_x != wrap_pos_x || latex->scale != g.FontSize * scale) {
            latex->scale = g.FontSize * scale;
            latex->src = src_view;
            latex->wrap_pos_x = wrap_pos_x;
            latex->image = std::make_unique<Latex::LatexImage>(
                latex->src, g.FontSize * scale,
                wrap_pos_x >= 0 ? wrap_pos_x : 0, 7.f,
                ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
        }

        if (flags & ImGuiLatexFlags_Animate) {
            latex->animate = true;
        }

        if (latex->image->getLatexErrorMsg().empty()) {
            latex->animate = latex->image->render(ImVec2(1.f, 1.f), ImVec2(0.f, 0.f), latex->animate);
        } else {
            ImGui::Text("%s", latex->image->getLatexErrorMsg().c_str());
        }
    }
}