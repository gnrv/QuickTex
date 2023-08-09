#pragma once 
#include "window/draw_commands.h"
#include "state.h"
#include "char.h"
#include "core/basic.h"
#include "wrapper.h"

struct DrawableChar : public WrapCharacter {
    DrawableChar(Fonts::CharPtr char_ptr) {
        info = char_ptr;
    }
    bool virtual draw(Draw::DrawList* draw_list, const Rect& boundaries, ImVec2 draw_offset = ImVec2(0.f, 0.f)) = 0;
};

struct NewLine : DrawableChar {
    NewLine(Fonts::CharPtr char_ptr) : DrawableChar(char_ptr) {
        info->is_linebreak = true;
    }
    bool draw(Draw::DrawList* draw_list, const Rect& boundaries, ImVec2 draw_offset = ImVec2(0.f, 0.f)) override;
};

typedef DrawableChar* DrawableCharPtr;
