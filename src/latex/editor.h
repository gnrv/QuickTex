#pragma once

#include <string>
#include <map>
#include <set>
#include "tempo.h"
#include "cairo_painter.h"
#include "state.h"

#include "chars/im_char.h"
#include "window/draw_commands.h"

struct CharDecoInfo {
    enum Decoration { BACKGROUND, UNDERLINE, BOX, SQUIGLY };
    Decoration type;
    Colors::color color;
    float thickness = 1.f;
};

class LatexEditor {
private:
    std::string m_text;
    bool m_suggset = false;
    bool m_reparse = false;

    float m_line_space = 1.2f;

    WrapColumn m_wrap_column;
    std::map<size_t, size_t> m_commands;
    std::map<size_t, size_t> m_openings_to_closings;
    std::set<size_t> m_line_positions;
    Draw::DrawList m_draw_list;

    // Cursor related things
    float m_line_height = 15.f;
    float m_advance = 10.f;
    bool m_is_char_info_set = false;
    bool m_cursor_find_pos = true;
    size_t m_cursor_pos = 0;
    size_t m_cursor_selection_begin = 0;
    size_t m_cursor_line_number = 0;
    size_t m_cursor_last_hpos = 0;
    ImVec2 m_cursor_drawpos;
    float m_cursor_height;

    const int LEFT = 0x1;
    const int RIGHT = 0x2;
    const int UP = 0x4;
    const int DOWN = 0x8;
    int m_key_presses = 0;

    size_t find_line_begin(size_t pos);
    size_t find_line_end(size_t pos);
    size_t find_line_number(size_t pos);
    bool is_line_begin(size_t pos);

    void move_up(bool shift);
    void move_down(bool shift);
    void move_left(bool word = false, bool shift = false);
    void move_right(bool word = false, bool shift = false);
    void set_std_char_info();

    /**
     * @brief Modifies pos to next word
     *
     * @param pos
     * @return size_t return corresponding line_number
     */
    size_t find_next_word(size_t pos);
    size_t find_previous_word(size_t pos);
    size_t goto_next_bracket(size_t pos);
    size_t goto_previous_bracket(size_t pos);
    size_t goto_next_ampersand(size_t pos);
    size_t goto_previous_ampersand(size_t pos);

    bool is_in_texcommand();
    bool is_in_bracket();
    bool is_in_matrix();

    void home();
    void end();
    void select_word();

    ImVec2 find_char_placement(size_t pos, bool half_line_space = false);

    void draw_cursor();
    void char_decoration();
    void draw_decoration(ImVec2 char_p1, ImVec2 char_p2, const CharDecoInfo& decoration);
    void char_decoration(size_t from, size_t to, const std::vector<CharDecoInfo>& decorations);

    void keyboard_events();

    void click(const ImVec2& position);
    void double_click(const ImVec2& position);
    void triple_click(const ImVec2& position);

    void insert_at(size_t pos, const std::string& str);
    void delete_at(size_t from, size_t to);

    void parse();
public:
    void draw(std::string& latex, ImVec2 size = ImVec2(0, 0));

    void set_text(const std::string& text);
};