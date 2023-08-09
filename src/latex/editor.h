#pragma once

#include <string>
#include "tempo.h"
#include "cairo_painter.h"

class LatexEditor {
private:
    size_t m_cursor_pos = 0;
    size_t m_last_horizontal_pos = 0;
    std::string m_text;
    bool m_suggset = false;

    void move_up();
    void move_down();
    void move_left();
    void move_right();

    void goto_next_word();
    void goto_previous_word();
    void goto_next_bracket();
    void goto_previous_bracket();
    void goto_next_ampersand();
    void goto_previous_ampersand();

    bool is_in_texcommand();
    bool is_in_bracket();
    bool is_in_matrix();

    void home();
    void end();
    void select_word();

    void click(const ImVec2& position);
    void double_click(const ImVec2& position);
    void triple_click(const ImVec2& position);

    void insert_at(size_t pos, const std::string& str);
    void delete_at(size_t from, size_t to);

    void parse();
public:
    LatexEditor();
    ~LatexEditor();

    void draw();
};