#include <fstream>
#include <chrono>

#include "main_window.h"
#include "style.h"
#include "fonts/fonts.h"
#include "state.h"
#include "clip.h"
#include "latex/editor.h"

#include "imgui_stdlib.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// #include "microtex/lib/core/parser.h"
// #include "microtex/lib/core/formula.h"

void setFonts() {
    using namespace Fonts;
    using Fs = FontStyling;
    auto& state = UIState::getInstance();
    // Regular fonts
    state.font_manager.setFontPath(Fs{ F_REGULAR, W_REGULAR, S_NORMAL }, "data/fonts/Ubuntu/Ubuntu-R.ttf");
    state.font_manager.setFontPath(Fs{ F_REGULAR, W_REGULAR, S_ITALIC }, "data/fonts/Ubuntu/Ubuntu-RI.ttf");
    state.font_manager.setFontPath(Fs{ F_REGULAR, W_BOLD, S_NORMAL }, "data/fonts/Ubuntu/Ubuntu-B.ttf");
    state.font_manager.setFontPath(Fs{ F_REGULAR, W_BOLD, S_ITALIC }, "data/fonts/Ubuntu/Ubuntu-BI.ttf");

    // Monospace fonts
    state.font_manager.setFontPath(Fs{ F_MONOSPACE, W_REGULAR, S_NORMAL }, "data/fonts/Ubuntu/UbuntuMono-R.ttf");

    FontRequestInfo default_font;
    default_font.size_wish = 18.f;
    default_font.font_styling = Fs{ F_REGULAR, W_REGULAR, S_NORMAL };
    default_font.auto_scaling = true;
    state.font_manager.setDefaultFont(default_font);

    // merge in icons
    ImVector<ImWchar> icons_ranges;
    icons_ranges.push_back(static_cast<ImWchar>(ICON_MIN_MD));
    icons_ranges.push_back(static_cast<ImWchar>(ICON_MAX_MD));
    icons_ranges.push_back(static_cast<ImWchar>(0));
    ImFontConfig icons_config;
    // icons_config.PixelSnapH = true;
    icons_config.GlyphOffset = ImVec2(0, 4.f);
    icons_config.MergeMode = true;

    // Tempo::AddIconsToFont(state.font_regular,
    //     "data/fonts/Icons/material-design-icons/MaterialIcons-Regular.ttf", icons_config, icons_ranges
    // );
}
MainApp::MainApp() {

}
void MainApp::InitializationBeforeLoop() {
    setFonts();
    defineStyle();
}
void MainApp::AfterLoop() {
}

bool compare_floats(float* ptr1, float* ptr2, int size) {
    for (int i = 0;i < size;i++) {
        if (ptr1[i] != ptr2[i])
            return false;
    }
    return true;
}

float MainApp::check_time() {
    auto t = std::chrono::high_resolution_clock::now();
    auto t1 = std::chrono::duration_cast<std::chrono::milliseconds>(t - m_last_checkpoint).count();
    return (float)t1 / time_until_clipboard;
}

void MainApp::set_clipboard() {
    float fraction = check_time();
    if (fraction < 1.f || m_has_pasted)
        return;
    if (m_latex_image == nullptr || m_latex_image->getImage() == nullptr || m_latex_image->getImage()->width() == 0 || m_latex_image->getImage()->height() == 0)
        return;

    return;

    m_has_pasted = true;
    auto image = m_latex_image->getImage();
    clip::image_spec spec;
    spec.width = image->width();
    spec.height = image->height();
    spec.bits_per_pixel = 32;
    spec.bytes_per_row = spec.width * 4;
    spec.red_mask = 0x00ff0000;
    spec.green_mask = 0xff00;
    spec.blue_mask = 0xff;
    spec.alpha_mask = 0xff000000;
    spec.red_shift = 16;
    spec.green_shift = 8;
    spec.blue_shift = 0;
    spec.alpha_shift = 24;
    clip::image img(image->getData()->data(), spec);
    clip::set_image(img);
}
void MainApp::options() {
    if (ImGui::CollapsingHeader("Options:")) {
        // ImGui::Checkbox("Auto copy to clipboard", &m_copy_to_clipboard);
        ImGui::SetNextItemWidth(200);
        ImGui::DragInt("Font size", &m_font_size, 1.f, 4, 150);
        // if (ImGui::TreeNode("Advanced")) {
        ImGui::ColorEdit4("Text color", m_text_color);
        ImGui::ColorEdit3("Background color (for visualization)", m_background_color);
        //     ImGui::TreePop();
        // }
        ImGui::Separator();
    }
}
void MainApp::input_field(float width, float height) {
    // Input field
    Fonts::FontInfoOut font_out;
    Fonts::FontRequestInfo request;
    request.auto_scaling = true;
    request.font_styling = { Fonts::F_MONOSPACE, Fonts::W_REGULAR, Fonts::S_NORMAL };
    request.size_wish = 18;
    UIState::getInstance().font_manager.requestFont(request, font_out);
    Tempo::PushFont(font_out.font_id);
    ImGui::InputTextMultiline("##input", &m_txt, ImVec2(width - 10, height / 3.));
    Tempo::PopFont();

    // Progress bar
    if (m_latex_image != nullptr) {
        float fraction = check_time();
        if (fraction < 1) {
            ImGui::Text("Soon to be copied");
        }
        else {
            fraction = 1.f;
            ImGui::TextColored(ImVec4(0.f, 0.7f, 0.f, 1.f), "Copied to clipboard");
        }
        auto draw_list = ImGui::GetWindowDrawList();
        auto cursor_pos = ImGui::GetCursorScreenPos();
        float progress_bar_width = 150;
        draw_list->AddLine(
            ImVec2(cursor_pos.x, cursor_pos.y + 5),
            ImVec2(cursor_pos.x + progress_bar_width * fraction, cursor_pos.y + 5),
            ImGui::ColorConvertFloat4ToU32(ImVec4(0.f, 0.7f, 0.f, 1.f)),
            5.f);
        ImGui::Dummy(ImVec2(0, 10));
    }
}
void MainApp::generate_image() {
    // Generating tex image
    if (m_txt != m_prev_text || !compare_floats(m_text_color, m_prev_text_color, 4) || m_font_size != m_prev_font_size) {
        m_prev_text = m_txt;
        m_prev_font_size = m_font_size;
        m_prev_text_color[0] = m_text_color[0];
        m_prev_text_color[1] = m_text_color[1];
        m_prev_text_color[2] = m_text_color[2];
        m_prev_text_color[3] = m_text_color[3];
        m_latex_image = std::make_unique<Latex::LatexImage>(
            m_txt, (float)m_font_size * Tempo::GetScaling(),
            7.f,
            ImGui::ColorConvertFloat4ToU32(ImVec4(m_text_color[0], m_text_color[1], m_text_color[2], m_text_color[3])),
            ImVec2(1.f, 1.f), ImVec2(0.f, 0.f));

        // Copy to clipboard timer
        m_last_checkpoint = std::chrono::high_resolution_clock::now();
        m_has_pasted = false;

        // microtex::Formula formula(m_txt);

    }
}
void MainApp::result_window(float width) {
    // Result window
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(m_background_color[0], m_background_color[1], m_background_color[2], 1.f));
    auto avail = ImGui::GetContentRegionAvail();
    if (avail.y < 100)
        avail.y = 100;
    if (m_latex_image != nullptr && m_latex_image->getDimensions().y > avail.y)
        avail.y = m_latex_image->getDimensions().y;
    ImGui::BeginChild("##output", ImVec2(width - 10, avail.y));
    if (m_latex_image != nullptr) {
        if (m_latex_image->getLatexErrorMsg().empty()) {
            auto texture = m_latex_image->getImage()->texture();
            auto cursor_pos = ImGui::GetCursorScreenPos();
            auto draw_list = ImGui::GetWindowDrawList();
            draw_list->AddImage(
                m_latex_image->getImage()->texture(),
                cursor_pos,
                cursor_pos + m_latex_image->getDimensions()
            );
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.f, 0.f, 1.f));
            ImGui::Text(m_latex_image->getLatexErrorMsg().c_str());
            ImGui::PopStyleColor();
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

void MainApp::FrameUpdate() {
    /* ImGui configs */
    auto& io = ImGui::GetIO();
    float width = io.DisplaySize.x;
    float height = io.DisplaySize.y;

    ImGui::SetNextWindowSize(ImVec2(width, height));
    ImGui::SetNextWindowPos(ImVec2(0, 0));

    int flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("Text", 0, flags);
    options();
    input_field(width, height);
    result_window(width);
    ImGui::End();

}

void MainApp::BeforeFrameUpdate() {
    generate_image();
    set_clipboard();
}