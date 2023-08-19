#include "latex.h"
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

    void LatexImage::render(ImVec2 scale, ImVec2 inner_padding) {
        m_painter.start(ImVec2(ceil(m_render->getWidth()), ceil(m_render->getHeight())), scale, inner_padding);
        m_graphics.distributeCallList(&m_painter);
        m_painter.finish();
        auto data = m_painter.getImageData();
        if (data != nullptr)
            m_image->setImage(data, m_painter.getImageDimensions().x, m_painter.getImageDimensions().y, Image::FILTER_BILINEAR);
    }

    LatexImage::LatexImage(const std::string& latex_src, float font_size, float line_space, microtex::color text_color, ImVec2 scale, ImVec2 inner_padding) {
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
        if (m_latex_error_msg.empty())
            render(scale, inner_padding);
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

    void LatexImage::redraw(ImVec2 scale, ImVec2 inner_padding) {
        if (m_latex_error_msg.empty())
            render(scale, inner_padding);
    }
}