#pragma once

#include <imgui.h>
#include <implot.h>

namespace ImPlot {
    void Vector(const char* label_id, ImVec2 start, ImVec2 end, ImPlotItemFlags flags = ImPlotItemFlags_None);
}
