#include "imgui_scale.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <vector>

struct ImScaleData {
    float Scale{ 1.0 };
    ImVec2 WindowPadding;
    float WindowRounding;
    ImVec2 WindowMinSize;
    float ChildRounding;
    float PopupRounding;
    ImVec2 FramePadding;
    float FrameRounding;
    ImVec2 ItemSpacing;
    ImVec2 ItemInnerSpacing;
    ImVec2 CellPadding;
    ImVec2 TouchExtraPadding;
    float IndentSpacing;
    float ColumnsMinSpacing;
    float ScrollbarSize;
    float ScrollbarRounding;
    float GrabMinSize;
    float GrabRounding;
    float LogSliderDeadzone;
    float TabRounding;
    float TabMinWidthForCloseButton;
    float TabBarOverlineSize;
    ImVec2 SeparatorTextPadding;
    ImVec2 DisplayWindowPadding;
    ImVec2 DisplaySafeAreaPadding;
    float MouseCursorScale;

    ImScaleData() = default;
    ImScaleData(float scale) : Scale(scale) {
        ImGuiStyle& s = ImGui::GetStyle();
        WindowPadding = s.WindowPadding;
        WindowRounding = s.WindowRounding;
        WindowMinSize = s.WindowMinSize;
        ChildRounding = s.ChildRounding;
        PopupRounding = s.PopupRounding;
        FramePadding = s.FramePadding;
        FrameRounding = s.FrameRounding;
        ItemSpacing = s.ItemSpacing;
        ItemInnerSpacing = s.ItemInnerSpacing;
        CellPadding = s.CellPadding;
        TouchExtraPadding = s.TouchExtraPadding;
        IndentSpacing = s.IndentSpacing;
        ColumnsMinSpacing = s.ColumnsMinSpacing;
        ScrollbarSize = s.ScrollbarSize;
        ScrollbarRounding = s.ScrollbarRounding;
        GrabMinSize = s.GrabMinSize;
        GrabRounding = s.GrabRounding;
        LogSliderDeadzone = s.LogSliderDeadzone;
        TabRounding = s.TabRounding;
        TabMinWidthForCloseButton = s.TabMinWidthForCloseButton;
        TabBarOverlineSize = s.TabBarOverlineSize;
        SeparatorTextPadding = s.SeparatorTextPadding;
        DisplayWindowPadding = s.DisplayWindowPadding;
        DisplaySafeAreaPadding = s.DisplaySafeAreaPadding;
        MouseCursorScale = s.MouseCursorScale;
    }
};

ImScaleData g_ScaleData;
static ImVector<ImScaleData> g_ScaleDataStack;

void ImGui::PushScale(float scale) {
    g_ScaleDataStack.push_back(ImScaleData(g_ScaleData.Scale));
    ImGui::GetStyle().ScaleAllSizes(scale);
    ImGui::SetWindowFontScale(scale);
    g_ScaleData.Scale = scale;
}

float ImGui::GetScale() {
    return g_ScaleData.Scale;
}

void ImGui::PopScale() {
    ImScaleData &scaleData = g_ScaleDataStack.back();
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowPadding = scaleData.WindowPadding;
    s.WindowRounding = scaleData.WindowRounding;
    s.WindowMinSize = scaleData.WindowMinSize;
    s.ChildRounding = scaleData.ChildRounding;
    s.PopupRounding = scaleData.PopupRounding;
    s.FramePadding = scaleData.FramePadding;
    s.FrameRounding = scaleData.FrameRounding;
    s.ItemSpacing = scaleData.ItemSpacing;
    s.ItemInnerSpacing = scaleData.ItemInnerSpacing;
    s.CellPadding = scaleData.CellPadding;
    s.TouchExtraPadding = scaleData.TouchExtraPadding;
    s.IndentSpacing = scaleData.IndentSpacing;
    s.ColumnsMinSpacing = scaleData.ColumnsMinSpacing;
    s.ScrollbarSize = scaleData.ScrollbarSize;
    s.ScrollbarRounding = scaleData.ScrollbarRounding;
    s.GrabMinSize = scaleData.GrabMinSize;
    s.GrabRounding = scaleData.GrabRounding;
    s.LogSliderDeadzone = scaleData.LogSliderDeadzone;
    s.TabRounding = scaleData.TabRounding;
    s.TabMinWidthForCloseButton = scaleData.TabMinWidthForCloseButton;
    s.TabBarOverlineSize = scaleData.TabBarOverlineSize;
    s.SeparatorTextPadding = scaleData.SeparatorTextPadding;
    s.DisplayWindowPadding = scaleData.DisplayWindowPadding;
    s.DisplaySafeAreaPadding = scaleData.DisplaySafeAreaPadding;
    s.MouseCursorScale = scaleData.MouseCursorScale;
    g_ScaleData.Scale = scaleData.Scale;
    ImGui::SetWindowFontScale(scaleData.Scale);
    g_ScaleDataStack.pop_back();
}