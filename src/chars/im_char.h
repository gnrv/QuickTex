#pragma once

#include "core/colors.h"
#include "drawable_char.h"
#include "char.h"

/**
 * @brief Character for ImFont
 *
 */

struct Style {
    emfloat font_size = emfloat{ 20.f };
    Fonts::FontStyling font_styling = Fonts::FontStyling{ Fonts::F_REGULAR, Fonts::W_REGULAR, Fonts::S_NORMAL };
    bool font_underline = false;
    Colors::color font_color = Colors::white;
    Colors::color font_bg_color = Colors::transparent;
};

struct ImChar : public DrawableChar {
    Tempo::FontID m_font_id;
    float m_font_size;
    ImU32 m_color;
    ImWchar m_char;
public:
    ImChar(Tempo::FontID font_id, ImWchar c, float font_size, ImU32 color, Fonts::CharPtr char_ptr, int text_pos);

    bool draw(Draw::DrawList* draw_list, const Rect& boundaries, ImVec2 draw_offset = ImVec2(0.f, 0.f)) override;
};

bool Utf8StrToImCharStr(UIState& ui_state, WrapColumn* wrap_p, const std::string& str, int line, int start, int end, const Style& style, bool replace_spaces_by_points = false);
bool Utf8StrToImCharStr(UIState& ui_state, WrapString* wrap_str, const std::string& str, int line, int start, int end, const Style& style, bool replace_spaces_by_points = false);
