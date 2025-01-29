//#include <imga.h>

ImGui::Text("Insehkjhkrt ImPlot Here");

static float mk_size = ImPlot::GetStyle().MarkerSize;
static float mk_weight = ImPlot::GetStyle().MarkerWeight;
ImGui::SliderFloat("Size", &mk_size, 0, 10);
ImGui::SliderFloat("Weight", &mk_weight, 0, 10);
if (ImPlot::BeginPlot("hejz")) {
        ImPlot::SetupAxesLimits(0, 10, 0, 12);

        ImS8 xs[4] = {1,4,5,6};
        ImS8 ys[4] = {10,11,7,8};

        // filled markers
        for (int m = 0; m < ImPlotMarker_COUNT; ++m) {
            ImGui::PushID(m);
            ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 3);
            ImPlot::Vector("Hej", ImVec2(xs[0], ys[0]), ImVec2(xs[0]+2, ys[0]+mk_size), ImPlotItemFlags_NoLegend);
            ImPlot::PopStyleVar(1);
            ImGui::PopID();
            ys[0]--; ys[1]--;
            ys[2]--; ys[3]--;
        }
        xs[0] = 6; xs[1] = 9; ys[0] = 10; ys[1] = 11;
        // open markers
        for (int m = 0; m < ImPlotMarker_COUNT; ++m) {
            ImGui::PushID(m);
            ImPlot::SetNextMarkerStyle(m, mk_size, IMPLOT_AUTO_COL, mk_weight);
            ImPlot::PlotLine("##Filled", xs, ys, 2);
            ImGui::PopID();
            ys[0]--; ys[1]--;
        }

        ImPlot::PlotText("Vectors", 2.5f, 6.0f);
        ImPlot::PlotText("Open Markers",   7.5f, 6.0f);

        ImPlot::PushStyleColor(ImPlotCol_InlayText, ImVec4(1,0,1,1));
        ImPlot::PlotText("Vertical Text", 5.0f, 6.0f, ImVec2(0,0), ImPlotTextFlags_Vertical);
        ImPlot::PopStyleColor();

    ImPlot::EndPlot();
}

