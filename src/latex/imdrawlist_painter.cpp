#include "imdrawlist_painter.h"

#include <iostream>

#define DEBUG 0

using namespace microtex;

inline float max(float a, float b) {
    if (a > b)
        return a;
    return b;
}
inline float min(float a, float b) {
    if (a < b)
        return a;
    return b;
}

void ImDrawList_Painter::destroy() {
    m_draw_list = nullptr;
}

void ImDrawList_Painter::start(ImVec2 dimensions, ImVec2 scale, ImVec2 inner_padding) {
    destroy();
    m_painting = true;

    m_dimensions = ImVec2(
        int(scale.x * (dimensions.x + 2 * inner_padding.x)),
        int(scale.y * (dimensions.y + 2 * inner_padding.y))
    );
    m_scale = scale;
    m_offset = inner_padding;
    m_state = State();
    m_state_stack.clear();

    m_draw_list = ImGui::GetForegroundDrawList();

    m_origin = ImGui::GetCursorScreenPos();

    setColor(BLACK);
    setStroke(Stroke());
}

void ImDrawList_Painter::finish() {
    if (m_dimensions.x > 0 && m_dimensions.y > 0 && m_painting) {
        m_painting = false;
    }
}

ImVec2 ImDrawList_Painter::getRealPos(float x, float y) {
    // Transform x,y using m_state.dx, m_state.dy, m_state.sx, m_state.sy
    ImVec2 pos = ImVec2(x, y);
    pos.x = m_state.sx * pos.x + m_state.dx;
    pos.y = m_state.sy * pos.y + m_state.dy;
    return m_origin + ImVec2(m_scale.x * (pos.x + m_state.sx * m_offset.x), m_scale.y * (pos.y + m_state.sy * m_offset.y));
}

ImDrawList_Painter::ImDrawList_Painter() {
}

ImDrawList_Painter::~ImDrawList_Painter() {
    destroy();
}

void ImDrawList_Painter::setColor(color c) {
    // ImGui has same ARGB order as microtex
    m_state.color = c;
}

color ImDrawList_Painter::getColor() const {
    return m_state.color;
}

void ImDrawList_Painter::setStroke(const Stroke& s) {
    m_state.stroke = s;
    m_state.thickness = s.lineWidth * m_scale.x;

    // ImGui doesn't support line caps, joins or miter limit
}

void ImDrawList_Painter::setStrokeWidth(float w) {
    m_state.stroke.lineWidth = w;
    m_state.thickness = w * m_scale.x;
}

void ImDrawList_Painter::setDash(const std::vector<float>& dash) {
    // ImGui doesn't support dashes
}

void ImDrawList_Painter::setFont(const std::string&, float, int, const std::string&) {
}


void ImDrawList_Painter::setFontSize(float) {
}

void ImDrawList_Painter::translate(float dx, float dy) {
    #if DEBUG
    std::cout << "translate(" << dx << ", " << dy << ")" << std::endl;
    #endif
    // [[1 0 dx]    [[state.sx 0        state.dx]    [[state.sx 0        state.dx + dx]]
    //  [0 1 dy]  *  [0        state.sy state.dy]  =  [0        state.sy state.dy + dy]]
    //  [0 0 1 ]]    [0        0        1       ]]    [0        0        1               ]]

    // ImVec2 pos = ImVec2(dx, dy);
    // pos.x = m_state.sx * (pos.x + m_state.dx);
    // pos.y = m_state.sy * (pos.y + m_state.dy);
    // pos = ImVec2(1 * (pos.x + m_state.sx * 0), 1 * (pos.y + m_state.sy * 0));

    // m_state.dx += pos.x;
    // m_state.dy += pos.y;
    // return;

    m_state.dx += m_state.sx * dx;
    m_state.dy += m_state.sy * dy;
}

void ImDrawList_Painter::scale(float sx, float sy) {
    #if DEBUG
    std::cout << "scale(" << sx << ", " << sy << ")" << std::endl;
    #endif
    m_state.sx *= sx;
    m_state.sy *= sy;

    // [[sx 0  0]     [[state.sx 0        state.dx]    [[sx*state.sx 0        sx*state.dx]]
    //  [0  sy 0]  *   [0        state.sy state.dy]  =  [0        sy*state.sy sy*state.dy]]
    //  [0  0  1 ]]    [0        0        1       ]]    [0        0           1               ]]

    // m_state.dx *= sx;
    // m_state.dy *= sy;
}

void ImDrawList_Painter::rotate(float angle) {
}

void ImDrawList_Painter::rotate(float angle, float px, float py) {
}

void ImDrawList_Painter::reset() {
    m_state.sx = m_state.sy = 1.f;
    m_state.dx = m_state.dy = 0.f;
}

void ImDrawList_Painter::save() {
    m_state_stack.push_back(m_state);
}

void ImDrawList_Painter::restore() {
    m_state = m_state_stack.back();
    m_state_stack.pop_back();
}

void ImDrawList_Painter::drawGlyph(u16, float, float) {
}

void ImDrawList_Painter::beginPath(i32 id) {
    m_draw_list->PathClear();
}

void ImDrawList_Painter::moveTo(float x, float y) {
    if (m_draw_list->_Path.Size != 0) {
        m_path.push_back(SubPath());
        std::swap(m_draw_list->_Path, m_path.back().points);
    }
    ImVec2 pos = getRealPos(x, y);
    m_draw_list->PathLineTo(pos);
    pathSanityCheck();
}

void ImDrawList_Painter::lineTo(float x, float y) {
    ImVec2 pos = getRealPos(x, y);
    m_draw_list->PathLineTo(pos);
    pathSanityCheck();
}

void ImDrawList_Painter::cubicTo(float x1, float y1, float x2, float y2, float x3, float y3) {
    ImVec2 p2 = getRealPos(x1, y1);
    ImVec2 p3 = getRealPos(x2, y2);
    ImVec2 p4 = getRealPos(x3, y3);
    m_draw_list->PathBezierCubicCurveTo(p2, p3, p4);
    pathSanityCheck();
}

void ImDrawList_Painter::quadTo(float x1, float y1, float x2, float y2) {
    ImVec2 p2 = getRealPos(x1, y1);
    ImVec2 p3 = getRealPos(x2, y2);
    m_draw_list->PathBezierQuadraticCurveTo(p2, p3);
    pathSanityCheck();
}

void ImDrawList_Painter::closePath() {
    // The SubPath being built is stored in the m_draw_list->_Path,
    // so this is always operating on the current sub-path, not the first sub-path.
    if (m_draw_list->_Path.Size == 0)
        return;
    // NB: It is illegal to call push_back/push_front/insert with a reference pointing inside the ImVector data itself!
    //     we first need to copy it out.
    ImVec2 p = m_draw_list->_Path[0];
    m_draw_list->PathLineTo(p);
    pathSanityCheck();
}

void ImDrawList_Painter::fillPath(i32 id) {
#if DEBUG
    std::cout << "fillPath(";
    int subpath_i = 0;
#endif
    // Steal the last sub-path
    m_path.push_back(SubPath());
    std::swap(m_draw_list->_Path, m_path.back().points);
    // Then draw all the stolen sub-paths
    for (auto& sub_path : m_path) {
        std::swap(m_draw_list->_Path, sub_path.points);
#if DEBUG
        std::cout << "sub-path " << subpath_i++ << ": ";
        for (int i = 0; i < m_draw_list->_Path.Size; i++) {
            ImVec2 p = m_draw_list->_Path[i];
            std::cout << "[" << p.x << ", " << p.y << "], ";
        }
        std::cout << std::endl;
#endif
        m_draw_list->PathFillConcave(m_state.color);
    }
#if DEBUG
    std::cout << ")" << std::endl;
#endif
    m_path.clear();
}

void ImDrawList_Painter::strokePath(i32 id) {
    // Steal the last sub-path
    m_path.push_back(SubPath());
    std::swap(m_draw_list->_Path, m_path.back().points);
    // Then draw all the stolen sub-paths
    for (auto& sub_path : m_path) {
        std::swap(m_draw_list->_Path, sub_path.points);
        m_draw_list->PathStroke(m_state.color, ImDrawFlags_None, m_state.thickness * m_state.sy);
    }
    m_path.clear();
}

void ImDrawList_Painter::drawLine(float x1, float y1, float x2, float y2) {
    ImVec2 p1 = getRealPos(x1, y1);
    ImVec2 p2 = getRealPos(x2, y2);
    m_draw_list->AddLine(p1, p2, m_state.color, m_state.thickness * m_state.sy);
}

void ImDrawList_Painter::drawRect(float x, float y, float w, float h) {
    ImVec2 p1 = getRealPos(x, y);
    ImVec2 p2 = getRealPos(x + w, y + h);
    m_draw_list->AddRect(p1, p2, m_state.color, 0.f, ImDrawFlags_None, m_state.thickness * m_state.sy);
}

void ImDrawList_Painter::fillRect(float x, float y, float w, float h) {
    ImVec2 p1 = getRealPos(x, y);
    ImVec2 p2 = getRealPos(x + w, y + h);
    m_draw_list->AddRectFilled(p1, p2, m_state.color, 0.f, ImDrawFlags_None);
}

void ImDrawList_Painter::roundRect(float x, float y, float w, float h, float rx, float ry) {
    rx *= m_scale.x;
    ry *= m_scale.y;
    ImVec2 p1 = getRealPos(x, y);
    ImVec2 p2 = getRealPos(x + w, y + h);
    m_draw_list->PathRect(p1, p2, rx, ImDrawFlags_None);
    pathSanityCheck();
}

void ImDrawList_Painter::drawRoundRect(float x, float y, float w, float h, float rx, float ry) {
    rx *= m_scale.x;
    ry *= m_scale.y;
    ImVec2 p1 = getRealPos(x, y);
    ImVec2 p2 = getRealPos(x + w, y + h);
    m_draw_list->AddRect(p1, p2, m_state.color, rx, ImDrawFlags_None, m_state.thickness * m_state.sy);
}

void ImDrawList_Painter::fillRoundRect(float x, float y, float w, float h, float rx, float ry) {
    rx *= m_scale.x;
    ry *= m_scale.y;
    ImVec2 p1 = getRealPos(x, y);
    ImVec2 p2 = getRealPos(x + w, y + h);
    m_draw_list->AddRectFilled(p1, p2, m_state.color, rx, ImDrawFlags_None);
}

void ImDrawList_Painter::pathSanityCheck() {
    // Sometimes we end up with a path that ends with a spurious [0, 0], let's
    // put a breakpoint here and see what the path looks like when that happens.
    if (m_draw_list->_Path.Size > 0) {
        ImVec2 p = m_draw_list->_Path[m_draw_list->_Path.Size - 1];
        if (p.x < 1 && p.y < 1) {
            int i = 0;
        }
    }
}