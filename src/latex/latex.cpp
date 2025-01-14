#include "latex.h"
#include "imgui_internal.h"
#include <cmath>

namespace Latex {
    bool is_initialized = false;

    static std::string font_family = "Fira Math";
    static std::string font_name_math = "Fira Math Regular";

    std::string init() {
        using namespace microtex;

        microtex::MicroTeX::setRenderGlyphUsePath(true);
        try {
            const FontSrcFile fira_math("data/firamath/FiraMath-Regular.clm2", "data/firamath/FiraMath-Regular.otf");
            const FontSrcFile fira_sans_boldItalic("data/firasans/FiraSansOT-BoldItalic.clm2", "data/firasans/FiraSansOT-BoldItalic.otf");
            const FontSrcFile fira_sans_regular("data/firasans/FiraSansOT-Regular.clm2", "data/firasans/FiraSansOT-Regular.otf");
            const FontSrcFile fira_sans_bold("data/firasans/FiraSansOT-Bold.clm2", "data/firasans/FiraSansOT-Bold.otf");
            const FontSrcFile fira_sans_italic("data/firasans/FiraSansOT-RegularItalic.clm2", "data/firasans/FiraSansOT-RegularItalic.otf");
            // auto auto_font = microtex::InitFontSenseAuto();

            MicroTeX::init(fira_math);
            // MicroTeX::addFont(math_bold);
            MicroTeX::addFont(fira_sans_regular);
            MicroTeX::addFont(fira_sans_bold);
            MicroTeX::addFont(fira_sans_italic);
            MicroTeX::addFont(fira_sans_boldItalic);
            MicroTeX::setDefaultMainFont(font_family);
            MicroTeX::setDefaultMathFont(font_name_math);

            PlatformFactory::registerFactory("abstract", std::make_unique<PlatformFactory_abstract>());
            PlatformFactory::activate("abstract");
            is_initialized = true;
            return "";
        }
        catch (std::exception& e) {
            return e.what();
        }
    }

    bool isInitialized() {
        return is_initialized;
    }

    void release() {
        microtex::MicroTeX::release();
        is_initialized = false;
    }

    using microtex::Call;
    using microtex::Painter;
    using microtex::color;
    using microtex::Stroke;
    using microtex::u32;
    using microtex::i32;
    void visit(Painter *painter, const Call &call) {
        if (call.fct_name == "setColor") {
            painter->setColor(call.arguments[0].getData<color>());
        }
        else if (call.fct_name == "setStroke") {
            painter->setStroke(call.arguments[0].getData<Stroke>());
        }
        else if (call.fct_name == "setStrokeWidth") {
            painter->setStrokeWidth(call.arguments[0].getData<float>());
        }
        else if (call.fct_name == "setDash") {
            painter->setDash(call.arguments[0].getData<std::vector<float>>());
        }
        else if (call.fct_name == "setFont") {
            painter->setFont(
                call.arguments[0].getData<std::string>(),
                call.arguments[1].getData<float>(),
                call.arguments[2].getData<int>(),
                call.arguments[3].getData<std::string>()
            );
        }
        else if (call.fct_name == "setFontSize") {
            painter->setFontSize(call.arguments[0].getData<float>());
        }
        else if (call.fct_name == "translate") {
            painter->translate(call.arguments[0].getData<float>(), call.arguments[1].getData<float>());
        }
        else if (call.fct_name == "scale") {
            painter->scale(call.arguments[0].getData<float>(), call.arguments[1].getData<float>());
        }
        else if (call.fct_name == "rotate") {
            painter->rotate(call.arguments[0].getData<float>());
        }
        else if (call.fct_name == "rotateAroundPt") {
            painter->rotate(
                call.arguments[0].getData<float>(),
                call.arguments[1].getData<float>(),
                call.arguments[2].getData<float>()
            );
        }
        else if (call.fct_name == "reset") {
            painter->reset();
        }
        else if (call.fct_name == "drawGlyph") {
            painter->drawGlyph(
                call.arguments[0].getData<u32>(),
                call.arguments[1].getData<float>(),
                call.arguments[2].getData<float>()
            );
        }
        else if (call.fct_name == "beginPath") {
            painter->beginPath(call.arguments[0].getData<i32>());
        }
        else if (call.fct_name == "moveTo") {
            painter->moveTo(call.arguments[0].getData<float>(), call.arguments[1].getData<float>());
        }
        else if (call.fct_name == "lineTo") {
            painter->lineTo(call.arguments[0].getData<float>(), call.arguments[1].getData<float>());
        }
        else if (call.fct_name == "cubicTo") {
            painter->cubicTo(
                call.arguments[0].getData<float>(),
                call.arguments[1].getData<float>(),
                call.arguments[2].getData<float>(),
                call.arguments[3].getData<float>(),
                call.arguments[4].getData<float>(),
                call.arguments[5].getData<float>()
            );
        }
        else if (call.fct_name == "quadTo") {
            painter->quadTo(
                call.arguments[0].getData<float>(),
                call.arguments[1].getData<float>(),
                call.arguments[2].getData<float>(),
                call.arguments[3].getData<float>()
            );
        }
        else if (call.fct_name == "closePath") {
            painter->closePath();
        }
        else if (call.fct_name == "fillPath") {
            painter->fillPath(call.arguments[0].getData<i32>());
        }
        else if (call.fct_name == "drawLine") {
            painter->drawLine(
                call.arguments[0].getData<float>(),
                call.arguments[1].getData<float>(),
                call.arguments[2].getData<float>(),
                call.arguments[3].getData<float>()
            );
        }
        else if (call.fct_name == "drawRect") {
            painter->drawRect(
                call.arguments[0].getData<float>(),
                call.arguments[1].getData<float>(),
                call.arguments[2].getData<float>(),
                call.arguments[3].getData<float>()
            );
        }
        else if (call.fct_name == "fillRect") {
            painter->fillRect(
                call.arguments[0].getData<float>(),
                call.arguments[1].getData<float>(),
                call.arguments[2].getData<float>(),
                call.arguments[3].getData<float>()
            );
        }
        else if (call.fct_name == "drawRoundRect") {
            painter->drawRoundRect(
                call.arguments[0].getData<float>(),
                call.arguments[1].getData<float>(),
                call.arguments[2].getData<float>(),
                call.arguments[3].getData<float>(),
                call.arguments[4].getData<float>(),
                call.arguments[5].getData<float>()
            );
        }
        else if (call.fct_name == "fillRoundRect") {
            painter->fillRoundRect(
                call.arguments[0].getData<float>(),
                call.arguments[1].getData<float>(),
                call.arguments[2].getData<float>(),
                call.arguments[3].getData<float>(),
                call.arguments[4].getData<float>(),
                call.arguments[5].getData<float>()
            );
        }
    }
    /**
     * @brief Distributes the (functions) call list to the painter, with optional animation
     *
     * @param animate if true, the calls will be animated with a Manim-inspired animation
     * @return true if the animation is ongoing
     */
    bool distributeCallListManim(const std::vector<Call> &calls, Painter* painter, bool animate) {
        static int n = 1;
        if (!animate) {
            n = calls.size();
        }
        int i = 0;
        bool open_path = false;
        for (auto& call : calls) {
            if (call.fct_name == "beginPath") {
                open_path = true;
            } else if (call.fct_name == "fillPath") {
                open_path = false;
            }
            visit(painter, call);
            ++i;
            if (i == n) {
                break;
            }
        }
        if (open_path) {
            // painter->setColor(cyan);
            painter->setStroke(Stroke(20.f, microtex::CAP_ROUND, microtex::JOIN_ROUND));
            painter->strokePath(0);
        }
        if (i == calls.size()) {
            n = 1;
            return false; // animation finished
        }

        ++n;
        return true; // animation ongoing
    }

    /**
     * @brief Distributes the (functions) call list to the painter, with optional animation
     *
     * Animates by scaling and fading in each path and fading in (no scaling) each draw/fill call
     *
     * @param animate if true, the calls will be animated
     * @return true if the animation is ongoing
     */
    bool distributeCallListFadeIn(std::vector<Call> &calls, Painter* painter, bool animate) {
        static bool was_animating = false;
        static double t_start = 0;
        if (animate && !was_animating) {
            t_start = ImGui::GetTime();
        }
        int i = 0;
        constexpr double t_offset = 0.05;
        constexpr double t_scale = 4;
        bool all_saturated = true;
        for (auto& call : calls) {
            // Compute local interpolation parameter t, 0 <= t <= 1
            float t = ImSaturate(t_scale*(ImGui::GetTime() - t_start - i*t_offset));
            if (t < 1)
                all_saturated = false;
            if (call.fct_name == "beginPath") {
                painter->save(); // will be restored in fillPath
                painter->scale(0.5 + t/2, 0.5 + t/2);
                visit(painter, call);
            } else if (call.fct_name == "fillPath") {
                painter->setColor(microtex::color_fade(painter->getColor(), t));
                visit(painter, call);
                painter->restore(); // was saved in beginPath
                ++i;
            } else if (call.fct_name.rfind("draw", 0) == 0 || call.fct_name.rfind("fill", 0) == 0) {
                painter->setColor(microtex::color_fade(painter->getColor(), t));
                visit(painter, call);
                ++i;
            } else {
                visit(painter, call);
            }
        }

        was_animating = !all_saturated;
        return was_animating;
    }

    LatexImage::LatexImage(const std::string& latex_src,
                           float font_size, float width, float line_space,
                           microtex::color text_color) {
        if (!is_initialized) {
            m_latex_error_msg = "LateX has not been initialized";
            return;
        }
        using namespace microtex;
        try {
            std::locale::global(std::locale(""));
            // Default width large enough

            m_render = MicroTeX::parse(
                latex_src,
                width, font_size, line_space, text_color,
                true,
                OverrideTeXStyle(true, TexStyle::display),
                font_name_math
            );
            float height = m_render->getHeight(); // total height of the box = ascent + descent
            m_descent = m_render->getDepth();   // depth = descent
            m_ascent = height - m_descent;

            m_render->draw(m_graphics, 0.f, 0.f);
            // m_render->~Render();

        }
        catch (std::exception& e) {
            m_latex_error_msg = e.what();
        }
    }

    LatexImage::~LatexImage() {
        if (m_render != nullptr) {
            delete m_render;
        }
    }

    ImVec2 LatexImage::getDimensions() {
        if (m_latex_error_msg.empty())
            return ImVec2(ceil(m_render->getWidth()), ceil(m_render->getHeight()));
        else
            return ImVec2(0, 0);
    }

    bool LatexImage::render(ImVec2 scale, ImVec2 inner_padding, bool animate) {
        if (m_latex_error_msg.empty()) {
            m_painter.start(getDimensions(), scale, inner_padding);
            bool animating = distributeCallListFadeIn(m_graphics.getCallList(), &m_painter, animate);
            m_painter.finish();
            return animating;
        }

        return false;
    }
}
