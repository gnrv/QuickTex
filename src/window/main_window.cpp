#include <fstream>
#include <chrono>

#include "main_window.h"
#include "style.h"
#include "fonts/fonts.h"
#include "state.h"
#include "clip.h"

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
    state.font_manager.setFontPath(Fs{ F_REGULAR, W_REGULAR, S_NORMAL }, "data/fonts/ubuntu/Ubuntu-R.ttf");
    state.font_manager.setFontPath(Fs{ F_REGULAR, W_REGULAR, S_ITALIC }, "data/fonts/ubuntu/Ubuntu-RI.ttf");
    state.font_manager.setFontPath(Fs{ F_REGULAR, W_BOLD, S_NORMAL }, "data/fonts/ubuntu/Ubuntu-B.ttf");
    state.font_manager.setFontPath(Fs{ F_REGULAR, W_BOLD, S_ITALIC }, "data/fonts/ubuntu/Ubuntu-BI.ttf");

    // Monospace fonts
    state.font_manager.setFontPath(Fs{ F_MONOSPACE, W_REGULAR, S_NORMAL }, "data/fonts/ubuntu/UbuntuMono-R.ttf");

    FontRequestInfo default_font;
    default_font.size_wish = 18.f;
    default_font.font_styling = Fs{ F_REGULAR, W_REGULAR, S_NORMAL };
    default_font.auto_scaling = true;
    state.font_manager.setDefaultFont(default_font);

    // merge in icons
    ImVector<ImWchar> icons_ranges;
    icons_ranges.push_back(static_cast<ImWchar>(ICON_MIN_MD));
    icons_ranges.push_back(static_cast<ImWchar>(ICON_MAX_MD));
    state.font_manager.addIconsToFont(Fs{ F_REGULAR, W_REGULAR, S_NORMAL },
        "data/fonts/material-design-icons/MaterialIcons-Regular.ttf", icons_ranges
    );
}
MainApp::MainApp(const std::string& err) {
    m_err = err;
}
void MainApp::InitializationBeforeLoop() {
    setFonts();
    defineStyle();

    Tempo::Shortcut shortcut;
    shortcut.keys = { CMD_KEY, GLFW_KEY_S };
    shortcut.name = "Save to clipboard";
    shortcut.description = "Saves the image to the clipboard";
    shortcut.callback = [this]() {
        set_clipboard();
        };

    Tempo::Shortcut shortcut_savetofile;
    shortcut_savetofile.keys = { CMD_KEY, Tempo::KEY_SHIFT, GLFW_KEY_S };
    shortcut_savetofile.name = "Save to file";
    shortcut_savetofile.description = "Saves the image to a file";
    shortcut_savetofile.callback = [this]() {
        m_save_to_file = true;
        };

    Tempo::Shortcut bookmarks;
    bookmarks.keys = { CMD_KEY, GLFW_KEY_H };
    bookmarks.name = "History";
    bookmarks.description = "Shows the history";
    bookmarks.callback = [this]() {
        m_history.show();
        };

    Tempo::KeyboardShortCut::addShortcut(shortcut_savetofile);
    Tempo::KeyboardShortCut::addShortcut(shortcut);
    Tempo::KeyboardShortCut::addShortcut(bookmarks);

    // m_txt = "\\begin{align}\n"
    //     "(x+y)^3&=(x+y)(x+y)^2\\\\\n"
    //     "       &=(x+y)(x^2+2xy+y^2)\\\\\n"
    //     "       &=x^3+3x^2y+3xy^3+x^3.\n"
    //     "\\end{align}";
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
bool MainApp::is_valid() {
    return m_err.empty() && m_latex_image == nullptr && m_latex_image->getImage() == nullptr && m_latex_image->getImage()->width() == 0 && m_latex_image->getImage()->height() == 0;
}

void MainApp::set_clipboard() {
    if (!is_valid())
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

    m_history.saveToHistory({ m_txt, (float)image->width() / (float)image->height(), "" , m_inline, m_font_size });
}
void MainApp::save_to_file() {
    if (m_save_to_file) {
        m_save_to_file = false;

        if (!is_valid())
            return;

        NFD_Init();
        std::string filename;
        nfdchar_t* outPath;
        nfdfilteritem_t filterItem[1] = { { "PNG Image", "png" } };
        nfdresult_t result = NFD_SaveDialogU8(&outPath, filterItem, 1, NULL, NULL);
        if (result == NFD_OKAY) {
            filename = outPath;
            NFD_FreePathU8(outPath);
        }
        NFD_Quit();

        auto image = m_latex_image->getImage();
        stbi_write_png(filename.c_str(), image->width(), image->height(), 4, image->getData()->data(), image->width() * 4);
        m_just_saved_to_file = true;
        m_history.saveToHistory({ m_txt, (float)image->width() / (float)image->height(), "" , m_inline, m_font_size });
    }
}
void MainApp::options() {
    ImGui::SetNextItemWidth(200);
    ImGui::DragInt("Size", &m_font_size, 1.f, 4, 150);
    ImGui::SameLine();
    ImGui::Checkbox("Inline", &m_inline);
    if (ImGui::CollapsingHeader("Other options:")) {
        // ImGui::Checkbox("Auto copy to clipboard", &m_autocopy_to_clipboard);
        ImGui::ColorEdit4("Text color", m_text_color);
        ImGui::ColorEdit3("Background color (for visualization)", m_background_color);
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

    ImGui::Separator();
    m_latex_editor.draw(m_txt);
    ImGui::Separator();
    if (m_latex_editor.has_text_changed()) {
        m_txt = m_latex_editor.get_text();
    }

    // Progress bar or shortcut display
    if (m_latex_image != nullptr) {
        if (m_autocopy_to_clipboard) {
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
        else {
            if (m_has_pasted) {
                if (m_just_saved_to_file)
                    ImGui::Text("Saved to file");
                else
                    ImGui::Text("Copied to clipboard");
            }
            else {
                std::string cmd_name = Tempo::getKeyName(CMD_KEY);
                ImGui::Text("%s + S: copy to clipboard / %s + Shift + S: save to file / %s + H: History", cmd_name.c_str(), cmd_name.c_str(), cmd_name.c_str());
            }
        }
    }
}


void MainApp::generate_image() {
    // Generating tex image
    if (!m_err.empty())
        return;
    if (m_txt != m_prev_text || !compare_floats(m_text_color, m_prev_text_color, 4) || m_font_size != m_prev_font_size || m_inline != m_prev_inline) {
        m_prev_text = m_txt;
        m_prev_font_size = m_font_size;
        m_prev_inline = m_inline;
        m_prev_text_color[0] = m_text_color[0];
        m_prev_text_color[1] = m_text_color[1];
        m_prev_text_color[2] = m_text_color[2];
        m_prev_text_color[3] = m_text_color[3];
        std::string latex = m_txt;
        if (!m_inline) {
            latex = "\\[" + m_txt + "\\]";
        }
        m_latex_image = std::make_unique<Latex::LatexImage>(
            latex, (float)m_font_size * Tempo::GetScaling(),
            7.f,
            ImGui::ColorConvertFloat4ToU32(ImVec4(m_text_color[0], m_text_color[1], m_text_color[2], m_text_color[3])),
            ImVec2(1.f, 1.f), ImVec2(0.f, 0.f));

        // Copy to clipboard timer
        m_last_checkpoint = std::chrono::high_resolution_clock::now();
        m_has_pasted = false;
        m_just_saved_to_file = false;
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
        if (m_latex_image->getLatexErrorMsg().empty() && m_err.empty()) {
            auto texture = m_latex_image->getImage()->texture();
            auto cursor_pos = ImGui::GetCursorScreenPos();
            cursor_pos.x += 5;
            cursor_pos.y += 5;
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

    if (!m_err.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.f, 0.f, 1.f));
        ImGui::Text(m_err.c_str());
        ImGui::PopStyleColor();
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

    save_to_file();

    m_history.draw();
    if (m_history.is_open())
        m_latex_editor.set_focus(false);
    LatexHistory hist;
    if (m_history.must_retrieve_latex(hist)) {
        m_latex_editor.set_text(hist.latex);
        m_txt = hist.latex;
        m_font_size = hist.size;
        m_inline = hist.is_inline;
        m_prev_text = "";
    }
}

void MainApp::BeforeFrameUpdate() {
    generate_image();

    float fraction = check_time();
    if (fraction >= 1.f && !m_has_pasted && m_autocopy_to_clipboard)
        set_clipboard();
}