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
bool LatexEditor::is_line_begin(size_t pos) {
    return m_line_positions.find(pos) != m_line_positions.end();
}

/* ===============================
 *             Events
 * =============================== */
void LatexEditor::set_last_key_pressed(int key) {
    if (m_last_key_pressed == key) {
        m_last_key_pressed_counter++;
    }
    else
        m_last_key_pressed_counter = 0;
    m_last_key_pressed = key;
}
void LatexEditor::events(const Rect& boundaries) {
    auto& io = ImGui::GetIO();
    int current_key_presses = ImGui::IsKeyPressed(ImGuiKey_LeftArrow, true) ? K_LEFT : 0
        | ImGui::IsKeyPressed(ImGuiKey_RightArrow, true) ? K_RIGHT : 0
        | ImGui::IsKeyPressed(ImGuiKey_UpArrow, true) ? K_UP : 0
        | ImGui::IsKeyPressed(ImGuiKey_DownArrow, true) ? K_DOWN : 0
        | ImGui::IsKeyPressed(ImGuiKey_Home, true) ? K_HOME : 0
        | ImGui::IsKeyPressed(ImGuiKey_End, true) ? K_END : 0
        | ImGui::IsKeyPressed(ImGuiKey_Delete, true) ? K_DEL : 0
        | ImGui::IsKeyPressed(ImGuiKey_Backspace, true) ? K_BACKSPACE : 0
        | ImGui::IsKeyPressed(ImGuiKey_Tab, true) ? K_TAB : 0
        | ImGui::IsKeyPressed(ImGuiKey_Enter, true) ? K_ENTER : 0
        | ImGui::IsKeyPressed(ImGuiKey_Escape, true) ? K_ESCAPE : 0;
    int new_key_presses = current_key_presses & ~m_key_presses;
    int to_press = 0;
    if (m_is_focused) {
        // Keyboard events
        if (new_key_presses != 0) {
            to_press = new_key_presses;
        }
        else if (current_key_presses) {
            to_press = current_key_presses;
        }
        if (to_press & K_LEFT) {
            move_left(io.KeyCtrl, io.KeyShift);
            set_last_key_pressed(K_LEFT);
        }
        else if (to_press & K_RIGHT) {
            move_right(io.KeyCtrl, io.KeyShift);
            set_last_key_pressed(K_RIGHT);
        }
        else if (to_press & K_UP) {
            move_up(io.KeyShift);
            set_last_key_pressed(K_UP);
        }
        else if (to_press & K_DOWN) {
            move_down(io.KeyShift);
            set_last_key_pressed(K_DOWN);
        }
        else if (to_press & K_HOME) {
            home(io.KeyShift);
            set_last_key_pressed(K_HOME);
        }
        else if (to_press & K_END) {
            end(io.KeyShift);
            set_last_key_pressed(K_END);
        }
        else if (to_press & K_BACKSPACE) {
            if (m_cursor_pos != m_cursor_selection_begin) {
                delete_at(m_cursor_selection_begin, m_cursor_pos, false);

            }
            else if (m_cursor_pos > 0) {
                if (io.KeyCtrl)
                    delete_at(find_previous_word(m_cursor_pos), m_cursor_pos, false);
                else
                    delete_at(m_cursor_pos - 1, m_cursor_pos, false);
            }
        }
        else if (to_press & K_DEL) {
            if (m_cursor_pos != m_cursor_selection_begin) {
                delete_at(m_cursor_selection_begin, m_cursor_pos, false);
            }
            else if (m_cursor_pos < m_text.size()) {
                if (io.KeyCtrl)
                    delete_at(m_cursor_pos, find_next_word(m_cursor_pos), false);
                else
                    delete_at(m_cursor_pos, m_cursor_pos + 1, false);
            }
        }
        else if (to_press & K_ENTER) {
            std::string to_insert = "\n";
            insert_with_selection(to_insert);
        }
        else if (io.InputQueueCharacters.Size > 0) {
            std::string to_insert;
            for (int n = 0; n < io.InputQueueCharacters.Size; n++) {
                unsigned int c = (unsigned int)io.InputQueueCharacters[n];
                if (c == '\t' && io.KeyShift)
                    continue;
                to_insert += c;
            }
            insert_with_selection(to_insert);
            // Consume characters
            io.InputQueueCharacters.resize(0);
        }
        // Todo: proper shortcuts, with cmd or ctrl
        else if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_A)) {
            m_cursor_selection_begin = 0;
            m_cursor_pos = m_text.size();
            m_cursor_last_hpos = m_cursor_pos - find_line_begin(m_cursor_pos);
            m_cursor_find_pos = true;
        }
        else if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_D)) {
            select_word();
        }
    }

    auto mouse_pos = ImGui::GetMousePos();
    auto cursor_pos = ImGui::GetCursorScreenPos();
    auto relative_pos = mouse_pos - cursor_pos;

    if (isInsideRect(mouse_pos, boundaries)) {
        if (ImGui::IsMouseClicked(0)) {
            m_is_focused = true;
            m_cursor_pos = coordinate_to_charpos(relative_pos);
            if (!io.KeyShift)
                m_cursor_selection_begin = m_cursor_pos;
            m_cursor_line_number = find_line_number(m_cursor_pos);
            m_cursor_last_hpos = m_cursor_pos - find_line_begin(m_cursor_pos);
            m_cursor_find_pos = true;
        }
        else if (ImGui::IsMouseDragging(0)) {
            m_cursor_pos = coordinate_to_charpos(relative_pos);
            m_cursor_line_number = find_line_number(m_cursor_pos);
            m_cursor_last_hpos = m_cursor_pos - find_line_begin(m_cursor_pos);
            m_cursor_find_pos = true;
        }
        else if (ImGui::IsMouseDoubleClicked(0)) {
            std::cout << "Double clicked" << std::endl;
            select_word();
        }
    }
    else {
        if (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1) || ImGui::IsMouseClicked(2)) {
            m_is_focused = false;
        }
    }
    // else if (Imgui::IsKeyPressed(ImGuiKey_Delete)) {
    //     if (ImGui::GetIO().KeyCtrl)
    //         delete_at(m_cursor_pos, find_next_word(m_cursor_pos));
    //     else
    //         delete_at(m_cursor_pos, m_cursor_pos + 1);
    // }
    // else if (ImGui::IsKeyPressed())
    //     m_key_presses = current_key_presses;
    // }
    // else {
    //     m_key_presses = 0;
    // }
}


/* ===============================
 *            Parsing
 * =============================== */
void LatexEditor::insert_with_selection(const std::string& str) {
    if (m_cursor_selection_begin != m_cursor_pos) {
        delete_at(m_cursor_selection_begin, m_cursor_pos, true);
        if (m_cursor_pos > m_cursor_selection_begin)
            m_cursor_pos = m_cursor_selection_begin;
    }
    insert_at(m_cursor_pos, str, false);
}
void LatexEditor::insert_at(size_t pos, const std::string& str, bool skip_cursor_move) {
    m_text.insert(pos, str);
    m_has_text_changed = true;
    if (!skip_cursor_move) {
        m_cursor_pos += str.size();
        parse();
        m_cursor_selection_begin = m_cursor_pos;
        m_cursor_line_number = find_line_number(m_cursor_pos);
        m_cursor_last_hpos = m_cursor_pos - find_line_begin(m_cursor_pos);
        m_cursor_find_pos = true;
    }
}
void LatexEditor::delete_at(size_t from, size_t to, bool skip_cursor_move) {
    if (from > to)
        std::swap(from, to);
    m_text.erase(from, to - from);
    m_has_text_changed = true;
    if (!skip_cursor_move) {
        parse();
        m_cursor_pos = from;
        m_cursor_selection_begin = m_cursor_pos;
        m_cursor_line_number = find_line_number(m_cursor_pos);
        m_cursor_last_hpos = m_cursor_pos - find_line_begin(m_cursor_pos);
        m_cursor_find_pos = true;
    }
}
void LatexEditor::parse() {
    m_reparse = false;
    m_wrap_column.clear();
    m_line_positions.clear();
    m_commands.clear();
    m_openings_to_closings.clear();

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

            style.font_color = m_config.command_color;
            m_reparse |= !Utf8StrToImCharStr(ui_state, &m_wrap_column, m_text, line_count, command_start, i, style, false);
            i--;
        }
        else if (m_text[i] == '{' || m_text[i] == '}') {
            if (m_text[i] == '{')
                bracket_counter++;
            else
                bracket_counter--;
            style.font_color = m_config.bracket_color;
            m_reparse |= !Utf8StrToImCharStr(ui_state, &m_wrap_column, m_text, line_count, i, i + 1, style, false);
        }
        else {
            style.font_color = m_config.text_color;
            m_reparse |= !Utf8StrToImCharStr(ui_state, &m_wrap_column, m_text, line_count, i, i + 1, style, false);
        }
    }
    WrapAlgorithm wrapper;
    wrapper.setWidth(5000000, false);
    wrapper.setLineSpace(m_line_space, false);
    wrapper.setTextColumn(&m_wrap_column);
}

void LatexEditor::set_text(const std::string& text) {
    m_text = text;
    parse();
}

/* ===============================
 *       Text cursor movement
 * =============================== */

void LatexEditor::move_up(bool shift) {
    if (m_cursor_line_number > 0) {
        m_cursor_find_pos = true;
        m_cursor_pos = find_line_begin(m_cursor_pos);
        m_cursor_pos--;
        size_t line_begin = find_line_begin(m_cursor_pos);
        while (m_cursor_pos > 0 && m_cursor_last_hpos < m_cursor_pos - line_begin) {
            m_cursor_pos--;
        }
        m_cursor_line_number = find_line_number(m_cursor_pos);
    }
    else if (m_cursor_pos > 0) {
        m_cursor_pos = 0;
        m_cursor_find_pos = true;
        m_cursor_last_hpos = 0;
    }
    if (!shift)
        m_cursor_selection_begin = m_cursor_pos;
}
void LatexEditor::move_down(bool shift) {
    if (m_cursor_line_number < m_line_positions.size() - 1) {
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
        m_cursor_line_number = find_line_number(m_cursor_pos);
    }
    else if (m_cursor_pos < m_text.size()) {
        m_cursor_pos = m_text.size();
        m_cursor_last_hpos = m_cursor_pos - find_line_begin(m_cursor_pos);
        m_cursor_find_pos = true;
    }
    if (!shift)
        m_cursor_selection_begin = m_cursor_pos;
}
void LatexEditor::move_left(bool word, bool shift) {
    if (m_cursor_pos > 0) {
        if (m_cursor_selection_begin != m_cursor_pos && !shift) {
            if (m_cursor_pos > m_cursor_selection_begin)
                m_cursor_pos = m_cursor_selection_begin;
            else
                m_cursor_selection_begin = m_cursor_pos;
        }
        else {
            if (m_line_positions.find(m_cursor_pos) != m_line_positions.end()) {
                m_cursor_pos--;
            }
            else if (word) {
                m_cursor_pos = find_previous_word(m_cursor_pos);
            }
            else {
                m_cursor_pos--;
            }
            m_cursor_line_number = find_line_number(m_cursor_pos);
            m_cursor_last_hpos = m_cursor_pos - find_line_begin(m_cursor_pos);
            m_cursor_find_pos = true;
        }
    }
    if (!shift)
        m_cursor_selection_begin = m_cursor_pos;
}
void LatexEditor::move_right(bool word, bool shift) {
    if (m_cursor_pos < m_text.size()) {
        if (m_cursor_selection_begin != m_cursor_pos && !shift) {
            if (m_cursor_pos > m_cursor_selection_begin)
                m_cursor_pos = m_cursor_selection_begin;
            else
                m_cursor_selection_begin = m_cursor_pos;
        }
        else {
            if (m_line_positions.find(m_cursor_pos + 1) != m_line_positions.end()) {
                m_cursor_pos++;
            }
            else if (word) {
                m_cursor_pos = find_next_word(m_cursor_pos);
            }
            else {
                m_cursor_pos++;
            }
            m_cursor_line_number = find_line_number(m_cursor_pos);
            m_cursor_last_hpos = m_cursor_pos - find_line_begin(m_cursor_pos);
            m_cursor_find_pos = true;
        }
    }
    if (!shift)
        m_cursor_selection_begin = m_cursor_pos;
}
void LatexEditor::home(bool shift) {
    bool skip_whitespace = true;
    if (m_last_key_pressed == K_HOME) {
        skip_whitespace = false;
    }
    m_cursor_pos = find_home(m_cursor_pos, skip_whitespace);
    m_cursor_last_hpos = m_cursor_pos - find_line_begin(m_cursor_pos);
    m_cursor_find_pos = true;
    if (!shift)
        m_cursor_selection_begin = m_cursor_pos;
}
void LatexEditor::end(bool shift) {
    bool skip_whitespace = true;
    m_cursor_pos = find_end(m_cursor_pos, skip_whitespace);
    m_cursor_last_hpos = m_cursor_pos - find_line_begin(m_cursor_pos);
    m_cursor_find_pos = true;
    if (!shift)
        m_cursor_selection_begin = m_cursor_pos;
}
void LatexEditor::select_word() {
    m_cursor_selection_begin = find_previous_word(m_cursor_pos);
    m_cursor_pos = find_next_word(m_cursor_pos);
    m_cursor_last_hpos = m_cursor_pos - find_line_begin(m_cursor_pos);
    m_cursor_find_pos = true;
}


/* ===============================
 *     Text cursor primitives
 * =============================== */
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
size_t LatexEditor::find_home(size_t pos, bool skip_whitespace) {
    size_t line_begin = find_line_begin(pos);
    size_t line_end = find_line_end(pos);
    pos = line_begin;
    if (skip_whitespace) {
        while (is_whitespace(m_text[pos]) && pos <= line_end)
            pos++;
        if (pos == line_end)
            pos = line_begin;
    }
    return pos;
}
size_t LatexEditor::find_end(size_t pos, bool) {
    size_t line_begin = find_line_begin(pos);
    size_t line_end = find_line_end(pos);
    pos = line_end;
    return pos;
}

/* ===============================
 *             Drawing
 * =============================== */
void LatexEditor::set_std_char_info() {
    if (!m_is_char_info_set) {
        WrapColumn col;
        std::string test_string = "abc|&g";
        auto& ui_state = UIState::getInstance();
        Style style;
        m_is_char_info_set |= Utf8StrToImCharStr(ui_state, &col, test_string, 0, 0, 2, style, false);
        if (m_is_char_info_set) {
            WrapAlgorithm wrapper;
            wrapper.setWidth(5000000, false);
            wrapper.setLineSpace(m_line_space, false);
            wrapper.setTextColumn(&col);
            auto& subline = col[0].sublines.front();
            m_line_height = subline.max_ascent + subline.max_descent;
            m_advance = col[0].chars.front()->info->advance;
        }
    }
}

ImVec2 LatexEditor::find_char_placement(size_t pos, bool half_line_space) {
    size_t line_number = find_line_number(pos);
    auto& line = m_wrap_column[line_number];
    bool found_next_char = false;
    float last_x_pos = 0.f;
    ImVec2 out_pos(0, line.relative_y_pos);
    for (auto ch : line.chars) {
        if (ch->text_position > pos) {
            out_pos.x = ch->calculated_position.x;
            found_next_char = true;
            break;
        }
        last_x_pos = ch->calculated_position.x + ch->info->advance;
    }
    if (!found_next_char) {
        out_pos.x = last_x_pos;
    }
    if (half_line_space) {
        out_pos.y -= m_line_height * (m_line_space - 1.f) / 2.f;
    }
    return out_pos;
}
void LatexEditor::draw_cursor() {
    if (m_cursor_find_pos && m_wrap_column.find(m_cursor_line_number) != m_wrap_column.end()) {
        m_cursor_find_pos = false;
        m_cursor_drawpos = find_char_placement(m_cursor_pos, true);
    }
    ImVec2 pos = ImGui::GetCursorScreenPos() + m_cursor_drawpos;
    pos.x -= 1.f;
    ImVec2 end_pos = pos;
    end_pos.y += m_line_height * m_line_space;
    m_draw_list->AddLine(pos, end_pos, m_config.cursor_color, 1.f);
}
void LatexEditor::draw_decoration(ImVec2 char_p1, ImVec2 char_p2, const CharDecoInfo& decoration) {
    if (decoration.type == CharDecoInfo::BACKGROUND)
        m_draw_list->AddRectFilled(char_p1, char_p2, decoration.color);
    else if (decoration.type == CharDecoInfo::UNDERLINE) {
        char_p1.y = char_p2.y;
        m_draw_list->AddLine(char_p1, char_p2, decoration.color, decoration.thickness.getFloat());
    }
    else if (decoration.type == CharDecoInfo::BOX) {
        m_draw_list->AddRect(char_p1, char_p2, decoration.color, 0.f, 0, decoration.thickness.getFloat());
    }
    else if (decoration.type == CharDecoInfo::SQUIGLY) {
        float x = char_p1.x;
        float y = char_p2.y;
        float w = char_p2.x - char_p1.x;
        float step = emfloat{ 4.f }.getFloat();
        float step_count = w / step;
        float step_height = emfloat{ 1.5f }.getFloat();
        float step_width = w / step_count;
        float current_y_step = step_height;
        for (float i = 0.f;i < step_count;i++) {
            if (x > char_p2.x)
                break;
            m_draw_list->AddLine(ImVec2(x, y - current_y_step), ImVec2(x + step_width, y + current_y_step), decoration.color, decoration.thickness.getFloat());
            x += step_width;
            current_y_step = -current_y_step;
        }
    }
}
void LatexEditor::char_decoration(size_t from, size_t to, const std::vector<CharDecoInfo>& decorations) {
    ImVec2 highlight_from;
    ImVec2 highlight_to;
    bool start_selection = false;
    auto screen_pos = ImGui::GetCursorScreenPos();
    for (size_t i = from; i < to;i++) {
        if (!start_selection) {
            start_selection = true;
            highlight_from = find_char_placement(i, true);
        }
        if (m_text[i] == '\n') {
            start_selection = false;
            highlight_to = find_char_placement(i, true);
            highlight_to.y += m_line_height * m_line_space;
            highlight_from += screen_pos;
            highlight_to += screen_pos;
            // Indicate to the user that he selected a \n
            highlight_to.x += m_advance / 3.f;
            for (const auto& deco : decorations) {
                draw_decoration(highlight_from, highlight_to, deco);
            }
        }
    }
    if (start_selection) {
        highlight_to = find_char_placement(to, true);
        highlight_to.y += m_line_height * m_line_space;
        highlight_from += screen_pos;
        highlight_to += screen_pos;
        for (const auto& deco : decorations) {
            draw_decoration(highlight_from, highlight_to, deco);
        }
    }
}

size_t LatexEditor::coordinate_to_charpos(const ImVec2& relative_coordinate) {
    if (m_wrap_column.empty())
        return 0;
    size_t line_number = 0;
    for (auto& pair : m_wrap_column) {
        if (pair.second.relative_y_pos > relative_coordinate.y)
            break;
        line_number++;
    }
    if (line_number > 0)
        line_number--;
    auto& line = m_wrap_column[line_number];
    float last_x_pos = 0.f;
    size_t position = 0;
    for (auto ch : line.chars) {
        if (ch->calculated_position.x + ch->info->advance / 2.f > relative_coordinate.x) {
            break;
        }
        position = ch->text_position;
    }
    return position;
}

void LatexEditor::debug_window() {
    if (m_config.debug) {
        ImGui::Begin("Debug");
        ImGui::Text("Cursor pos: %d", m_cursor_pos);
        ImGui::Text("Cursor line: %d", m_cursor_line_number);
        ImGui::Text("Cursor last h pos: %d", m_cursor_last_hpos);
        ImGui::Text("Is focused: %d", m_is_focused);
        ImGui::Text("Text size: %d", m_text.size());
        ImGui::End();
    }
}

void LatexEditor::draw(std::string& latex, ImVec2 size) {
    // formula.
    set_std_char_info();
    debug_window();

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
    boundaries.x = ImGui::GetCursorScreenPos().x;
    boundaries.y = ImGui::GetCursorScreenPos().y;
    boundaries.w = size.x;
    boundaries.h = size.y;
    events(boundaries);
    if (m_reparse) {
        parse();
    }

    m_draw_list.SetImDrawList(ImGui::GetWindowDrawList());

    size_t min_selection = std::min(m_cursor_pos, m_cursor_selection_begin);
    size_t max_selection = std::max(m_cursor_pos, m_cursor_selection_begin);
    draw_cursor();
    char_decoration(min_selection, max_selection, { { CharDecoInfo::BACKGROUND, m_config.selection_color } });

    for (auto& pair : m_wrap_column) {
        ImVec2 pos;
        for (auto& ptr : pair.second.chars) {
            auto p = std::static_pointer_cast<DrawableChar>(ptr);
            p->draw(&m_draw_list, boundaries, pos);
        }
    }
    ImGui::EndChild();
}

std::string LatexEditor::get_text() {
    return m_text;
}
bool LatexEditor::has_text_changed() {
    if (m_has_text_changed) {
        m_has_text_changed = false;
        return true;
    }
    else
        return false;
}