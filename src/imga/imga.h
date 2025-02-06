#pragma once

#include <imgui.h>
#include <implot.h>

enum ImPlotItemFlagsExtra_ {
    ImPlotItemFlags_NoLabel    = 1 << 10, // the item won't be labelled
};

namespace ImPlot {
    void Vector(const char* label_id, ImVec2 start, ImVec2 end, ImPlotItemFlags flags = ImPlotItemFlags_None);
    void Bivector(const char* label_id, ImVec2 start, ImVec2 mid, ImVec2 end, ImPlotItemFlags flags = ImPlotItemFlags_None);
}
