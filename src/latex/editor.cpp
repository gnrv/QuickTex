#include "editor.h"

#include "microtex/lib/core/formula.h"
#include "microtex/lib/core/parser.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
bool is_digit(char c) {
    return c >= '0' && c <= '9';
}
bool is_alphanum(char c) {
    return is_alpha(c) || is_digit(c);
}
bool is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

size_t LatexEditor::find_line_begin(size_t pos) {
    while (pos > 0 && m_text[pos - 1] != '\n')
        pos--;
    return pos;
}
size_t LatexEditor::find_line_end(size_t pos) {
    while (pos < m_text.size() && m_text[pos] != '\n')
        pos++;
    return pos;
}
size_t LatexEditor::find_line_number(size_t pos) {
    // Quick for tiny text
    size_t line_number = 0;
    for (auto it = std::next(m_line_positions.begin());it != m_line_positions.end();it++) {
        if (*it > pos)
            break;
        line_number++;
    }
    return line_number;
}
void LatexEditor::parse() {
    m_reparse = false;
    m_wrap_column.clear();

    auto& ui_state = UIState::getInstance();
    Style style;
    style.font_styling = { Fonts::F_MONOSPACE, Fonts::W_REGULAR, Fonts::S_NORMAL };
    style.font_color = Colors::black;
    style.font_bg_color = Colors::transparent;

    size_t line_start = 0;
    size_t line_end = 0;
    size_t line_count = 0;
    bool is_prev_backslash = false;
    size_t bracket_counter = 0;
    size_t left_counter = 0;
    m_line_positions.insert(0);

    for (size_t i = 0;i < m_text.size();i++) {
        is_prev_backslash = false;
        if (m_text[i] == '\n') {
            line_end = i;
            line_start = i;
            line_count++;
            m_line_positions.insert(i + 1);
        }
        else if (m_text[i] == '\\') {
            size_t command_start = i;
            i++;
            while (i < m_text.size() && is_alpha(m_text[i]))
                i++;
            if (i < m_text.size() - 1 && m_text[i + 1] == '\\')
                i++;

            style.font_color = Colors::lightslategray;
            m_reparse |= !Utf8StrToImCharStr(ui_state, &m_wrap_column, m_text, line_count, command_start, i, style, false);
            i--;
        }
        else if (m_text[i] == '{' || m_text[i] == '}') {
            if (m_text[i] == '{')
                bracket_counter++;
            else
                bracket_counter--;
            style.font_color = Colors::lightgray;
            m_reparse |= !Utf8StrToImCharStr(ui_state, &m_wrap_column, m_text, line_count, i, i + 1, style, false);
        }
        else {
            style.font_color = Colors::white;
            m_reparse |= !Utf8StrToImCharStr(ui_state, &m_wrap_column, m_text, line_count, i, i + 1, style, false);
        }
    }
    WrapAlgorithm wrapper;
    wrapper.setWidth(5000000, false);
    wrapper.setTextColumn(&m_wrap_column);
}

void LatexEditor::set_text(const std::string& text) {
    m_text = text;
    parse();
}

/* Cursor stuff */
void LatexEditor::move_up() {
    if (m_cursor_line_number > 0) {
        m_cursor_line_number--;
        m_cursor_find_pos = true;
        m_cursor_pos = find_line_begin(m_cursor_pos);
        m_cursor_pos--;
        size_t line_begin = find_line_begin(m_cursor_pos);
        while (m_cursor_pos > 0 && m_cursor_last_hpos < m_cursor_pos - line_begin) {
            m_cursor_pos--;
        }
    }
    else if (m_cursor_pos > 0) {
        m_cursor_pos = 0;
        m_cursor_find_pos = true;
        m_cursor_last_hpos = 0;
    }
}
void LatexEditor::move_down() {
    if (m_cursor_line_number < m_line_positions.size() - 1) {
        m_cursor_line_number++;
        m_cursor_find_pos = true;
        m_cursor_pos = find_line_end(m_cursor_pos);
        m_cursor_pos++;
        size_t line_begin = m_cursor_pos;
        while (m_cursor_pos < m_text.size() && m_cursor_last_hpos > m_cursor_pos - line_begin) {
            m_cursor_pos++;
        }
        if (m_cursor_pos > m_text.size()) {
            m_cursor_pos--;
        }
    }
    else if (m_cursor_pos < m_text.size()) {
        m_cursor_pos = m_text.size();
        m_cursor_last_hpos = m_cursor_pos - find_line_begin(m_cursor_pos);
        m_cursor_find_pos = true;
    }
}
void LatexEditor::move_left(bool word) {
    if (m_cursor_pos > 0) {
        if (m_line_positions.find(m_cursor_pos) != m_line_positions.end()) {
            m_cursor_line_number--;
            m_cursor_pos--;
        }
        else if (word) {
            m_cursor_pos = find_previous_word(m_cursor_pos);
            m_cursor_line_number = find_line_number(m_cursor_pos);
        }
        else {
            m_cursor_pos--;
        }
        m_cursor_last_hpos = m_cursor_pos - find_line_begin(m_cursor_pos);
        m_cursor_find_pos = true;
    }
}
void LatexEditor::move_right(bool word) {
    if (m_cursor_pos < m_text.size()) {
        if (m_line_positions.find(m_cursor_pos + 1) != m_line_positions.end()) {
            m_cursor_line_number++;
            m_cursor_pos++;
        }
        else if (word) {
            m_cursor_pos = find_next_word(m_cursor_pos);
            m_cursor_line_number = find_line_number(m_cursor_pos);
        }
        else {
            m_cursor_pos++;
        }
        m_cursor_last_hpos = m_cursor_pos - find_line_begin(m_cursor_pos);
        m_cursor_find_pos = true;
    }
}
size_t LatexEditor::find_next_word(size_t pos) {
    if (is_whitespace(m_text[pos])) {
        while (pos < m_text.size() && is_whitespace(m_text[pos]))
            pos++;
    }
    if (is_alphanum(m_text[pos])) {
        while (pos < m_text.size() && is_alphanum(m_text[pos]))
            pos++;
    }
    else {
        while (pos < m_text.size() && !is_alphanum(m_text[pos]) && !is_whitespace(m_text[pos]))
            pos++;
    }
    return pos;
}
size_t LatexEditor::find_previous_word(size_t pos) {
    if (pos < 2)
        return 0;

    pos--;
    if (is_whitespace(m_text[pos])) {
        while (pos > 0 && is_whitespace(m_text[pos]))
            pos--;
    }
    if (is_alphanum(m_text[pos])) {
        while (pos > 0 && is_alphanum(m_text[pos]))
            pos--;
    }
    else {
        while (pos > 0 && !is_alphanum(m_text[pos]) && !is_whitespace(m_text[pos]))
            pos--;
    }
    pos++;
    return pos;
}
void LatexEditor::set_line_height() {
    if (!m_is_line_height_set) {
        WrapColumn col;
        std::string test_string = "abc|&g";
        auto& ui_state = UIState::getInstance();
        Style style;
        m_is_line_height_set |= Utf8StrToImCharStr(ui_state, &col, test_string, 0, 0, 2, style, false);
        if (m_is_line_height_set) {
            WrapAlgorithm wrapper;
            wrapper.setWidth(5000000, false);
            wrapper.setTextColumn(&col);
            auto& subline = col[0].sublines.front();
            m_line_height = subline.max_ascent + subline.max_descent;
        }
    }
}
void LatexEditor::draw_cursor() {
    if (m_cursor_find_pos && m_wrap_column.find(m_cursor_line_number) != m_wrap_column.end()) {
        m_cursor_find_pos = false;
        auto& line = m_wrap_column[m_cursor_line_number];
        bool found_next_char = false;
        float last_x_pos = 0.f;
        m_cursor_drawpos = ImVec2(0, line.relative_y_pos);
        for (auto ch : line.chars) {
            if (ch->text_position > m_cursor_pos) {
                m_cursor_drawpos.x = ch->calculated_position.x;
                found_next_char = true;
                break;
            }
            last_x_pos = ch->calculated_position.x + ch->info->advance;
        }
        if (!found_next_char) {
            m_cursor_drawpos.x = last_x_pos;
        }
    }
    ImVec2 pos = ImGui::GetCursorScreenPos() + m_cursor_drawpos;
    pos.x -= 1.f;
    ImVec2 end_pos = pos;
    end_pos.y += m_line_height;
    m_draw_list->AddLine(pos, end_pos, Colors::lightgray, 1.f);
}

void LatexEditor::keyboard_events() {
    // if (ImGui::IsWindowFocused()) {
        // Last key pressed ?
        // Order of priority
    int left = ImGui::IsKeyPressed(ImGuiKey_LeftArrow, true) ? LEFT : 0;
    int right = ImGui::IsKeyPressed(ImGuiKey_RightArrow, true) ? RIGHT : 0;
    int up = ImGui::IsKeyPressed(ImGuiKey_UpArrow, true) ? UP : 0;
    int down = ImGui::IsKeyPressed(ImGuiKey_DownArrow, true) ? DOWN : 0;

    int current_key_presses = left | right | up | down;
    int new_key_presses = current_key_presses & ~m_key_presses;
    int to_press = 0;
    if (new_key_presses != 0) {
        to_press = new_key_presses;
    }
    else if (current_key_presses) {
        to_press = current_key_presses;
    }
    if (to_press & LEFT) {
        move_left(ImGui::GetIO().KeyCtrl);
    }
    else if (to_press & RIGHT) {
        move_right(ImGui::GetIO().KeyCtrl);
    }
    else if (to_press & UP) {
        move_up();
    }
    else if (to_press & DOWN) {
        move_down();
    }
    m_key_presses = current_key_presses;
    // }
    // else {
    //     m_key_presses = 0;
    // }
}

void LatexEditor::draw(std::string& latex, ImVec2 size) {
    // formula.
    set_line_height();
    keyboard_events();

    ImGui::Text("Linenum: %d, cursor_pos: %d", m_cursor_line_number, m_cursor_pos);
    ImGui::Text("Last hpos %d", m_cursor_last_hpos);

    if (size.y == 0) {
        size.y = emfloat{ 150 }.getFloat();
    }
    if (size.x == 0) {
        size.x = ImGui::GetContentRegionAvail().x;
    }
    ImGui::BeginChild("latex_editor", size, true);
    if (m_reparse) {
        parse();
    }
    Rect boundaries;
    boundaries.x = ImGui::GetCursorPosX();
    boundaries.y = ImGui::GetCursorPosY();
    boundaries.w = size.x;
    boundaries.h = size.y;

    m_draw_list.SetImDrawList(ImGui::GetWindowDrawList());

    for (auto& pair : m_wrap_column) {
        ImVec2 pos;
        for (auto& ptr : pair.second.chars) {
            auto p = std::static_pointer_cast<DrawableChar>(ptr);
            p->draw(&m_draw_list, boundaries, pos);
        }
    }
    draw_cursor();
    ImGui::EndChild();
}