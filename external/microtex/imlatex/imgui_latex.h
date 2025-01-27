#pragma once

#include <imgui.h>

typedef int ImGuiLatexFlags;       // -> enum ImGuiLatexFlags_     // Flags: for Latex() etc.

// If it capitalizes GUI as Gui, we'd better capitalize LaTeX as Latex
namespace ImGui {
    void InitLatex();
    void Latex(const char* src, ImGuiLatexFlags flags = 0);
}

// Flags for Latex() etc.
enum ImGuiLatexFlags_
{
    ImGuiLatexFlags_None               = 0,
};