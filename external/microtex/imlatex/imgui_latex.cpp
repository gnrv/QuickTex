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
    float font_size{ 16.f };
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
        if (latex->src != src_view || latex->wrap_pos_x != wrap_pos_x || latex->font_size != g.FontSize) {
            latex->font_size = g.FontSize;
            latex->src = src_view;
            latex->wrap_pos_x = wrap_pos_x;
            latex->image = std::make_unique<Latex::LatexImage>(
                latex->src, latex->font_size,
                wrap_pos_x >= 0 ? wrap_pos_x : 0, 7.f,
                ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
        }

        if (g.CurrentItemFlags & ImGuiItemFlags_Animated) {
            latex->animate = true;
        }

        if (latex->image->getLatexErrorMsg().empty()) {
            // TODO: We should only render if ItemAdd returns true
            latex->animate = latex->image->render(ImVec2(1.f, 1.f), ImVec2(0.f, 0.f), latex->animate);

            const ImVec2 text_pos(window->DC.CursorPos);
            const ImVec2 text_size(latex->image->getDimensions());
            ImRect bb(text_pos, text_pos + text_size);
            ItemSize(text_size, 0.0f);
            ItemAdd(bb, 0);
        } else {
            ImGui::Text("%s", latex->image->getLatexErrorMsg().c_str());
        }
    }
}