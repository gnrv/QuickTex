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
#include "defaults.h"

// Drawable and widgets
int TextInputCallback(ImGuiInputTextCallbackData* data);

class MainApp : public Tempo::App {
private:
    std::string m_txt;
    std::string m_prev_text;

    std::string m_current_path;

    DefaultParams m_defaults;
    DefaultParams m_prev_defaults;

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