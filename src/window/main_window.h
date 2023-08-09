#pragma once

#include <tempo.h>
#include <chrono>

#include "misc/cpp/imgui_stdlib.h"
#include "latex/latex.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

// Drawable and widgets
int TextInputCallback(ImGuiInputTextCallbackData* data);

class MainApp : public Tempo::App {
private:
    std::string m_txt;
    std::string m_prev_text;

    std::string m_current_path;
    bool m_autocopy_to_clipboard = false;
    int m_font_size = 50;
    int m_prev_font_size = 50;
    float m_text_color[4] = { 0.f, 0.f, 0.f, 1.f };
    float m_prev_text_color[4] = { 0.f, 0.f, 0.f, 1.f };
    float m_background_color[3] = { 1.f, 1.f, 1.f };

    float time_until_clipboard = 1500.f;
    std::chrono::high_resolution_clock::time_point m_last_checkpoint;
    bool m_has_pasted = true;

    std::unique_ptr<Latex::LatexImage> m_latex_image = nullptr;

    float check_time();

    void options();
    void input_field(float width, float height);
    void generate_image();
    void result_window(float width);
    void set_clipboard();
public:
    MainApp();
    virtual ~MainApp() {}

    void InitializationBeforeLoop() override;

    void AfterLoop() override;
    void FrameUpdate() override;
    void BeforeFrameUpdate() override;
};