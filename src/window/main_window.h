#pragma once

#include <tempo.h>
#include <chrono>

#include "misc/cpp/imgui_stdlib.h"
#include "latex/latex.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include "state.h"
#include "latex/editor.h"
#include "latex/history.h"

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
    bool m_inline = false;
    bool m_prev_inline = false;
    int m_family_idx = 0;
    int m_prev_family_idx = 0;

    float time_until_clipboard = 1500.f;
    std::chrono::high_resolution_clock::time_point m_last_checkpoint;
    bool m_has_pasted = true;
    bool m_save_to_file = false;
    bool m_just_saved_to_file = false;

    History m_history;

    LatexEditor m_latex_editor;

    std::string m_err;
    std::unique_ptr<Latex::LatexImage> m_latex_image = nullptr;

    float check_time();

    bool is_valid();

    void options();
    void input_field(float width, float height);
    void generate_image();
    void result_window(float width);
    void set_clipboard();
    void save_to_file();
public:
    MainApp(const std::string& err);
    virtual ~MainApp() {}

    void InitializationBeforeLoop() override;

    void AfterLoop() override;
    void FrameUpdate() override;
    void BeforeFrameUpdate() override;
};