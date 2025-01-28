ImGui::Text("Insehkjhkrt ImPlot Here");

if (ImPlot::BeginPlot("hejz")) {
        ImPlot::SetupAxesLimits(0, 10, 0, 12);

        ImS8 xs[4] = {1,4,5,6};
        ImS8 ys[4] = {10,11,7,8};

        // filled markers
        for (int m = 0; m < ImPlotMarker_COUNT; ++m) {
            ImGui::PushID(m);
            ImPlot::SetNextMarkerStyle(m, mk_size, IMPLOT_AUTO_COL, mk_weight);
            ImPlot::PlotLine("##Filled", xs, ys, 4);
            ImGui::PopID();
            ys[0]--; ys[1]--;
            ys[2]--; ys[3]--;
        }
        xs[0] = 6; xs[1] = 9; ys[0] = 10; ys[1] = 11;
        // open markers
        for (int m = 0; m < ImPlotMarker_COUNT; ++m) {
            ImGui::PushID(m);
            ImPlot::SetNextMarkerStyle(m, mk_size, ImVec4(0,0,0,0), mk_weight);
            ImPlot::PlotLine("##Open", xs, ys, 2);
            ImGui::PopID();
            ys[0]--; ys[1]--;
        }

        ImPlot::PlotText("Filled Markers", 2.5f, 6.0f);
        ImPlot::PlotText("Open Markers",   7.5f, 6.0f);

        ImPlot::PushStyleColor(ImPlotCol_InlayText, ImVec4(1,0,1,1));
        ImPlot::PlotText("Vertical Text", 5.0f, 6.0f, ImVec2(0,0), ImPlotTextFlags_Vertical);
        ImPlot::PopStyleColor();

    ImPlot::EndPlot();
}
