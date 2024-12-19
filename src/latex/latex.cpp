#include "latex.h"
#include "imgui_internal.h"
#include <cmath>

namespace Latex {
    bool is_initialized = false;

    static std::string font_family = "XITS";
    static std::string font_family_math = "XITS Math";

    std::string init(const std::string& family) {
        using namespace microtex;

        microtex::MicroTeX::setRenderGlyphUsePath(true);
        try {
            const FontSrcFile tex_gyre("data/tex-gyre/texgyredejavu-math.clm2", "data/tex-gyre/texgyredejavu-math.otf");
            const FontSrcFile latin_modern("data/lm-math/latinmodern-math.clm2", "data/lm-math/latinmodern-math.otf");
            const FontSrcFile fira_math("data/firamath/FiraMath-Regular.clm2", "data/firamath/FiraMath-Regular.otf");
            // XITS
            const FontSrcFile math_regular("data/xits/XITSMath-Regular.clm2", "data/xits/XITSMath-Regular.otf");
            // const FontSrcFile math_bold("data/xits/XITSMath-Bold.clm2", "data/xits/XITSMath-Bold.otf");
            const FontSrcFile xits_boldItalic("data/xits/XITS-BoldItalic.clm2", "data/xits/XITS-BoldItalic.otf");
            const FontSrcFile xits_regular("data/xits/XITS-Regular.clm2", "data/xits/XITS-Regular.otf");
            const FontSrcFile xits_bold("data/xits/XITS-Bold.clm2", "data/xits/XITS-Bold.otf");
            const FontSrcFile xits_italic("data/xits/XITS-Italic.clm2", "data/xits/XITS-Italic.otf");
            // auto auto_font = microtex::InitFontSenseAuto();

            MicroTeX::init(math_regular);
            // MicroTeX::addFont(math_bold);
            MicroTeX::addFont(xits_boldItalic);
            MicroTeX::addFont(xits_regular);
            MicroTeX::addFont(xits_bold);
            MicroTeX::addFont(xits_italic);
            MicroTeX::addFont(tex_gyre);
            MicroTeX::addFont(latin_modern);
            MicroTeX::addFont(fira_math);
            MicroTeX::setDefaultMainFont("XITS");
            MicroTeX::setDefaultMathFont("XITS");

            PlatformFactory::registerFactory("abstract", std::make_unique<PlatformFactory_abstract>());
            PlatformFactory::activate("abstract");
            is_initialized = true;
            return "";
        }
        catch (std::exception& e) {
            return e.what();
        }
    }
    std::vector<std::string> getFontFamilies() {
        return { "Latin Modern", "XITS" ,  /*"XITS Bold" // Is broken, */ "Fira Math",  "Gyre DejaVu" };
    }
    void setDefaultFontFamily(const std::string& family) {
        using namespace microtex;
        if (family == "XITS") {
            font_family_math = "XITS Math";
        }
        // else if (family == "XITS Bold") {
        //     font_family_math = "XITS Math Bold";
        // }
        else if (family == "Fira Math") {
            font_family_math = "Fira Math Regular";
        }
        else if (family == "Latin Modern") {
            font_family_math = "LatinModernMath-Regular";
        }
        else if (family == "Gyre DejaVu") {
            font_family_math = "TeXGyreDejaVuMath-Regular";
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
     * @brief Distributes the (functions) call list to the painter
     *
     * @param painter must be a child of Painter
     * @param animate if true, the calls will be animated
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

    bool distributeCallListFadeIn(const std::vector<Call> &calls, Painter* painter, bool animate) {
        static int n = 1;
        if (!animate) {
            n = calls.size();
        }
        int i = 0;
        static bool fading = false;
        static int fading_n = 0;
        static float fading_t = 0;
        float t = 0;
        static int fading_path_i = 0;
        int path_i = 0;
        if (fading) {
            fading_t += ImGui::GetIO().DeltaTime;
            t = ImSaturate(4*fading_t);
        }
        for (auto& call : calls) {
            if (call.fct_name == "beginPath") {
                path_i = i;
                if (fading && i == fading_path_i) {
                    painter->scale(t, t);
                }
                visit(painter, call);
            }
            if (call.fct_name == "fillPath") {
                if (!fading && i > fading_n && i == n - 1) {
                    fading = true;
                    fading_n = i;
                    fading_path_i = path_i;
                    fading_t = 0;
                } else if (fading && i == fading_n) {
                    using namespace microtex;
                    color c = painter->getColor();
                    c = argb(t*color_a(c)/255.f, color_r(c)/255.f, color_g(c)/255.f, color_b(c)/255.f);
                    painter->setColor(c);
                    visit(painter, call);
                    if (fading_t > 0.25) {
                        fading = false;
                    }
                } else {
                    visit(painter, call);
                }
            } else {
                visit(painter, call);
            }
            ++i;
            if (i == n) {
                break;
            }
        }
        if (i == calls.size()) {
            n = 1;
            return false; // animation finished
        }

        if (!fading)
            ++n;
        return true; // animation ongoing
    }

    bool LatexImage::render(ImVec2 scale, ImVec2 inner_padding, bool animate) {
        m_painter.start(ImVec2(ceil(m_render->getWidth()), ceil(m_render->getHeight())), scale, inner_padding);
        bool animating = distributeCallListFadeIn(m_graphics.getCallList(), &m_painter, animate);
        m_painter.finish();
        auto data = m_painter.getImageData();
        if (data != nullptr)
            m_image->setImage(data, m_painter.getImageDimensions().x, m_painter.getImageDimensions().y, Image::FILTER_BILINEAR);
        return animating;
    }

    LatexImage::LatexImage(const std::string& latex_src, float font_size, float line_space, microtex::color text_color) {
        if (!is_initialized) {
            m_latex_error_msg = "LateX has not been initialized";
            return;
        }
        m_image = std::make_shared<Image>();
        using namespace microtex;
        try {
            std::locale::global(std::locale(""));
            // Default width large enough

            m_render = MicroTeX::parse(
                latex_src,
                0, font_size, line_space, text_color,
                true,
                OverrideTeXStyle(false, TexStyle::display),
                font_family_math
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

    std::shared_ptr<Image> LatexImage::getImage() {
        return m_image;
    }

    ImVec2 LatexImage::getDimensions() {
        if (m_latex_error_msg.empty())
            return m_painter.getImageDimensions();
        else
            return ImVec2(0, 0);
    }

    void LatexImage::forgetImage() {
        m_image->reset();
    }

    bool LatexImage::redraw(ImVec2 scale, ImVec2 inner_padding, bool animate) {
        if (m_latex_error_msg.empty())
            return render(scale, inner_padding, animate);

        return false;
    }
}
