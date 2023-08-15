#pragma once

#include <string>
#include <map>
#include <set>
#include "tempo.h"
#include "cairo_painter.h"
#include "state.h"

#include "chars/im_char.h"
#include "window/draw_commands.h"
#include "search/commands.h"

struct CharDecoInfo {
    enum Decoration { BACKGROUND, UNDERLINE, BOX, SQUIGLY };
    Decoration type;
    Colors::color color;
    emfloat thickness{ 1.f };
};

struct LatexEditorConfig {
    float text_size = 18.f;
    float line_space = 1.2f;
    Colors::color text_color = Colors::white;
    Colors::color command_color = Colors::lightslategray;
    Colors::color bracket_color = Colors::lightgray;
    Colors::color background_color = Colors::black;
    Colors::color cursor_color = Colors::lightgray;
    Colors::color selection_color = ImGui::ColorConvertFloat4ToU32(ImVec4(0.5f, 0.5f, 0.5f, 0.5f));
    Colors::color charbox_color = Colors::white;
    Colors::color error_color = Colors::red;
    bool debug = false;
};

struct HistoryPoint {
    enum InsertType { ALPHANUM, WHITESPACE, SPECIAL, NONE };
    std::string text;
    size_t cursor_pos;
    InsertType insert_type = NONE;
};

class LatexEditor {
private:
    std::string m_text;
    bool m_suggset = false;
    bool m_reparse = false;
    bool m_is_focused = true;

    float m_line_space = 1.2f;

    WrapColumn m_wrap_column;
    std::map<size_t, size_t> m_commands;
    std::map<size_t, size_t> m_openings_to_closings;
    std::set<size_t> m_line_positions;
    Draw::DrawList m_draw_list;

    // Suggestions
    Search::CommandSearch m_search_commands;
    bool m_start_suggesting = false;
    std::vector<Search::Command> m_search_results;
    size_t m_search_highlight = 0;
    std::string m_query;

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
    bool m_start_dragging = false;

    // Calculated quantities
    float m_total_height = 0.f;
    float m_total_width = 0.f;

    // Editor configurations
    LatexEditorConfig m_config;

    // History
    std::vector<HistoryPoint> m_history = { { "", 0 } };
    int m_history_idx = 0;

    // Events
    const int K_LEFT = 0x1;
    const int K_RIGHT = 0x2;
    const int K_UP = 0x4;
    const int K_DOWN = 0x8;
    const int K_HOME = 0x10;
    const int K_END = 0x20;
    const int K_DEL = 0x40;
    const int K_BACKSPACE = 0x80;
    const int K_TAB = 0x100;
    const int K_ENTER = 0x200;
    const int K_CTRL = 0x400;
    const int K_ESCAPE = 0x800;

    int m_key_presses = 0;
    int m_last_key_pressed = 0;
    int m_last_key_pressed_counter = 0;
    bool m_has_text_changed = false;

    inline void set_last_key_pressed(int key);
    void events(const Rect& boundaries);

    size_t find_line_begin(size_t pos);
    size_t find_line_end(size_t pos);
    size_t find_line_number(size_t pos);
    bool is_line_begin(size_t pos);

    void move_up(bool shift);
    void move_down(bool shift);
    void move_left(bool word = false, bool shift = false);
    void move_right(bool word = false, bool shift = false);
    void home(bool shift);
    void end(bool shift);
    void select_word();
    void set_cursor_idx(size_t pos, bool remove_selection);

    /**
     * @brief Modifies pos to next word
     *
     * @param pos
     * @return size_t return corresponding line_number
     */
    size_t find_next_word(size_t pos);
    size_t find_previous_word(size_t pos);
    size_t find_home(size_t pos, bool skip_whitespace);
    size_t find_end(size_t pos, bool skip_whitespace);

    size_t goto_next_bracket(size_t pos);
    size_t goto_previous_bracket(size_t pos);
    size_t goto_next_ampersand(size_t pos);
    size_t goto_previous_ampersand(size_t pos);

    bool is_in_texcommand();
    bool is_in_bracket();
    bool is_in_matrix();

    ImVec2 locate_char_coord(size_t pos, bool half_line_space = false);
    void set_std_char_info();
    void draw_cursor();
    void char_decoration();
    void draw_decoration(ImVec2 char_p1, ImVec2 char_p2, const CharDecoInfo& decoration);
    void char_decoration(size_t from, size_t to, const std::vector<CharDecoInfo>& decorations);
    void draw_suggestions();

    size_t coordinate_to_charpos(const ImVec2& relative_coordinate);

    void click(const ImVec2& position);
    void double_click(const ImVec2& position);
    void triple_click(const ImVec2& position);

    enum HistoryAction { FORCE, GUESS, NONE };
    void make_history(HistoryAction action, const std::string& text_cpy, size_t cursor_pos, size_t change_size, char start_char);
    void insert_with_selection(const std::string& str);
    void insert_at(size_t pos, const std::string& str, bool skip_cursor_move, HistoryAction action = HistoryAction::GUESS);
    void delete_at(size_t from, size_t to, bool skip_cursor_move, HistoryAction action = HistoryAction::GUESS);
    void parse();

    void debug_window();
public:
    void draw(std::string& latex, ImVec2 size = ImVec2(0, 0));

    void set_text(const std::string& text);
    std::string get_text();
    bool has_text_changed();
    void set_focus(bool focus) { m_is_focused = focus; }
};