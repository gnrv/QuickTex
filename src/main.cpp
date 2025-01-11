#include <iostream>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "IconsMaterialDesignIcons.h"

#include "TextEditor.h"

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

#include <string>

#include <chrono>
#include <iostream>
#include <exception>

#include "latex/latex.h"

#include <chrono>
#include <filesystem>
#include <fstream>

#ifdef USE_CLING
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "cling/Interpreter/Interpreter.h"
#include "cling/MetaProcessor/InputValidator.h"
#endif

#include "system/sys_util.h"

#define ENABLE_CODE_WINDOW 1

int main(int argc, char **argv) {
    std::filesystem::current_path(getExecutablePath());

    std::string err = Latex::init();

#ifdef USE_CLING
    cling::Interpreter interp(argc, argv);

    // The interpreter has so much internal state going on in order to support incremental parsing,
    // I dont' dare to use it for syntax checking even. Let's create another interpreter for that.
    // We need to pass -fsyntax-only to the compiler.
    // std::vector<const char*> syntax_argv = {argv[0], "-fsyntax-only"};
    // cling::Interpreter syntax(argc, syntax_argv.data());

    // auto result = interp.loadLibrary("libimgui.so");
    // if (result != cling::Interpreter::kSuccess) {
    //     std::cerr << "Failed to load imgui library: " << result << std::endl;
    //     exit(1);
    // }
    // Add the imgui source directory to the include path
    interp.AddIncludePath("../external/imgui/imgui");
    //syntax.AddIncludePath("../external/imgui/imgui");
    // Pre-include it
    auto result = interp.loadHeader("imgui.h");
    if (result != cling::Interpreter::kSuccess) {
        std::cerr << "Failed to load imgui header: " << result << std::endl;
        exit(1);
    }
    //syntax.loadHeader("imgui.h");

    // Tell cling to allow re-definitions
    interp.getRuntimeOptions().AllowRedefinition = true;
#else
    (void)argc;
    (void)argv;
#endif

    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

    // On WSL2, the scale returned is 1.0, even though Windows is using a scale of 200%.
    // Watch out, because WSLg might be just blowing up the window to 2x! Resulting in a
    // nastly pixellated look. Our framebuffer is still just 1x.
    // Get rid of that totally fake scaling using
    // $ cat /mnt/c/Users/<user>/.wslgconfig
    // [system-distro-env]
    // WESTON_RDP_DEBUG_DESKTOP_SCALING_FACTOR=100
    float dpi_scale = 1.f;
    float window_size_scale_factor = 1.f;
    bool is_wsl2 = false;
    std::ifstream file("/proc/version");
    if (file.good()) {
        std::string line;
        std::getline(file, line);
        if (line.find("WSL") != std::string::npos) {
            // WSL2 detected
            is_wsl2 = true;
            dpi_scale = 2.f; // Or whatever your setting is in Windows, e.g. 200% is 2.f
            window_size_scale_factor = 1.f; // For the window size
        }
    }

    // To determine possible window height, query the monitor height.
    int monitor_count;
    GLFWmonitor **monitors = glfwGetMonitors(&monitor_count);
    int window_height = 720;
    if (monitor_count) {
        int w, h;
        glfwGetMonitorWorkarea(monitors[0], NULL, NULL, &w, &h);
        // Now find the greatest "p" we can have in 720p, 1080p etc
        if (h > 1440) {
            window_height = 1440;
        } else if (h > 1080) {
            window_height = 1080;
        } else if (h > 720) {
            window_height = 720;
        }

        if (!is_wsl2)
            glfwGetMonitorContentScale(monitors[0], NULL, &dpi_scale);
    }
    printf("Content scale: %f\n", dpi_scale);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(16*window_height*window_size_scale_factor/10,
                                          window_height*window_size_scale_factor,
                                          "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(dpi_scale);

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    ImFont *fira_sans = io.Fonts->AddFontFromFileTTF("../data/fonts/fira/FiraSans-Regular.ttf", 16.0f*dpi_scale);
    (void)fira_sans; // We don't use this font, it's the default

    ImFontConfig config;
    config.MergeMode = true;
    config.GlyphMinAdvanceX = 16.0f; // Use if you want to make the icon monospaced
    static const ImWchar icon_ranges[] = { ICON_MIN_MDI, ICON_MAX_MDI, 0 };
    io.Fonts->AddFontFromFileTTF("../data/fonts/material-design-icons/materialdesignicons-webfont.ttf", 16.0f*dpi_scale, &config, icon_ranges);

    ImFont *fira_mono = io.Fonts->AddFontFromFileTTF("../data/fonts/fira/FiraMono-Regular.ttf", 16.0f*dpi_scale);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    //ImVec4 clear_color = ImVec4(0.f, 0.f, 0.f, 1.00f);

    // Set up some math in microTex
    std::string latex{ R"(\begin{gather}
\gamma_\mu\gamma_\nu+\gamma_\nu\gamma_\mu=2\eta_{\mu\nu}\\
\mathbf{\sigma}_i = \gamma_i \gamma_0\\
\nabla\psi I\mathbf{\sigma}_3=m\psi\gamma_0\\
i\hat{\gamma}_\mu \frac{\partial}{\partial x^{\mu}} |\psi\rangle = m|\psi\rangle
\end{gather})" };

    float slide_font_size = 32.f; // px
    float slide_effective_font_size = 0;
    auto latex_image = std::make_unique<Latex::LatexImage>(
        latex, slide_effective_font_size,
        7.f,
        ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
    bool animate_latex = true;

    TextEditor editor;
    auto lang = TextEditor::LanguageDefinition::CPlusPlus();
    editor.SetLanguageDefinition(lang);
    editor.SetImGuiChildIgnored(true);

    bool editor_text_cromulent = true;
#if 0
    static const char* fileToEdit = "../src/main.cpp";
    {
        std::ifstream t(fileToEdit);
        if (t.good())
        {
            std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
            editor.SetText(str);
        }
    }
#else
    editor.SetText("ImGui::Text(\"Hello, world!\");");

    // Extract the text into an C++ function and compile it
#if 0
    // Bug with hole punching and background color
    editor.SetText(R"(
\newcolumntype{s}{>{\color{#1234B6}}c}
\begin{array}{|c|c|c|s|}
  \hline
  \rowcolor{Tan}\multicolumn{4}{|c|}{\textcolor{white}{\bold{\text{Table Head}}}}\\
  \hline
  \text{Matrix}&\multicolumn{2}{|c|}{\text{Multicolumns}}&\text{Font size commands}\\
  \hline
  \begin{pmatrix}
      \alpha_{11}&\cdots&\alpha_{1n}\\
      \hdotsfor{3}\\
      \alpha_{n1}&\cdots&\alpha_{nn}
  \end{pmatrix}
  &\large \text{Left}&\cellcolor{#00bde5}\small \textcolor{white}{\text{\bold{Right}}}
  &\small \text{small Small}\\
  \hline
  \multicolumn{4}{|c|}{\text{Table Foot}}\\
  \hline
\end{array}
)");
#endif
#endif

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Background windows:
        auto& io = ImGui::GetIO();
        float width = io.DisplaySize.x;
        float height = io.DisplaySize.y;
        int flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus |
                    ImGuiWindowFlags_NoNavFocus |
                    ImGuiWindowFlags_NoSavedSettings;

        // 1. Code window
#if ENABLE_CODE_WINDOW
        ImGui::SetNextWindowSize(ImVec2(width/2, height));
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::Begin("Code", 0, flags | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Save"))
                {
                    auto textToSave = editor.GetText();
                    /// save text....
                }
                if (ImGui::MenuItem("Quit", "Alt-F4"))
                    break;
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                bool ro = editor.IsReadOnly();
                if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
                    editor.SetReadOnly(ro);
                ImGui::Separator();

                if (ImGui::MenuItem("Undo", "Ctrl-Z", nullptr, !ro && editor.CanUndo()))
                    editor.Undo();
                if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && editor.CanRedo()))
                    editor.Redo();

                ImGui::Separator();

                if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))
                    editor.Copy();
                if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && editor.HasSelection()))
                    editor.Cut();
                if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && editor.HasSelection()))
                    editor.Delete();
                if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
                    editor.Paste();

                ImGui::Separator();

                if (ImGui::MenuItem("Select all", nullptr, nullptr))
                    editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View"))
            {
                if (ImGui::MenuItem("Dark palette"))
                    editor.SetPalette(TextEditor::GetDarkPalette());
                if (ImGui::MenuItem("Light palette"))
                    editor.SetPalette(TextEditor::GetLightPalette());
                if (ImGui::MenuItem("Retro blue palette"))
                    editor.SetPalette(TextEditor::GetRetroBluePalette());
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::PushFont(fira_mono);

        // Disable the status bar, it causes scrollview content height to increase
#if 0
        auto cpos = editor.GetCursorPosition();
        ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
            editor.IsOverwrite() ? "Ovr" : "Ins",
            editor.CanUndo() ? "*" : " ",
            editor.GetLanguageDefinition().mName.c_str(), fileToEdit);
#endif

        editor.Render("TextEditor");

        ImGui::PopFont();
        ImGui::End();

        static bool first = true;
        if (first || editor.IsTextChanged()) {
            // clang::SyntaxOnlyAction action;
            // // if (action.BeginSourceFile(syntax.getCI(), syntax.getCI()->getFrontendOpts())) {
            // //     action.Execute();
            // //     action.EndSourceFile();
            // // }
            // editor_text_cromulent = syntax.getCI()->ExecuteAction(action);
            // auto result = syntax.process(editor.GetText());
            // editor_text_cromulent = result == cling::Interpreter::kSuccess;
            cling::InputValidator validator;
            auto result = validator.validate(editor.GetText());
            editor_text_cromulent = result == cling::InputValidator::kComplete;
            first = false;
        }
#endif // CODE WINDOW

        // 2. Presentation window
        ImGui::SetNextWindowSize(ImVec2(width/2, height));
        ImGui::SetNextWindowPos(ImVec2(width/2, 0));
        // Get rid of horizontal padding
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
        // Transparent scrollbar bg
        // Doesn't work, the scrollbar always affects clip rect and layout
        // if we want an overlaid scroll bar, we need to draw it ourselves
        // So disable the built in scrollbar completely
        flags |= ImGuiWindowFlags_NoScrollbar;
        // Let the presentation area contain 10 placeholder slides
        // of aspect ratio 16:10
        ImGui::Begin("Presentation", 0, flags);

        // Slides are designed for 1080p, 16:10 aspect ratio
        ImVec2 slide_size{ width/2, width/2*10/16 };
        float slide_scale = slide_size.y / 1080.f;

        // Re-render the latex image if effective font size changed
        if (slide_effective_font_size != slide_scale*slide_font_size*dpi_scale || editor.IsTextChanged()) {
            latex = editor.GetText();
            // TODO: An option is to render at high resolution and then scale down the quad we render the texture on.
            slide_effective_font_size = slide_scale*slide_font_size*dpi_scale;
            latex_image = std::make_unique<Latex::LatexImage>(
                latex, slide_effective_font_size,
                7.f,
                ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
            animate_latex = true;
        }

        for (int i = 0; i < 10; i++) {
            ImGui::PushID(i);
            ImGui::BeginChild("Slide", slide_size, false);
            auto top_left = ImGui::GetCursorScreenPos();
            ImGui::Text("Slide %d", i);
            ImGui::SameLine();
            ImGui::SetCursorPosX(slide_size.x - ImGui::CalcTextSize(ICON_MDI_REFRESH).x - ImGui::GetStyle().FramePadding.x * 2);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            if (ImGui::Button(ICON_MDI_REFRESH))
                animate_latex = true;
            ImGui::PopStyleColor(1);
            // The problem here is that the drawlist uses window coordinates.
            // We need to convert the coordinates to window coordinates.
            // We can do this by using the cursor position.
            ImGui::GetWindowDrawList()->AddRect(top_left, top_left + slide_size, IM_COL32(255, 255, 255, 255));
            // if (i == 0) {
            //     if (latex_image->getLatexErrorMsg().empty()) {
            //         animate_latex = latex_image->render(ImVec2(1.f, 1.f), ImVec2(0.f, 0.f), animate_latex);
            //     } else {
            //         ImGui::Text("%s", latex_image->getLatexErrorMsg().c_str());
            //     }
            // }
            if (i == 0) {
                static bool wait_for_new_text = false;
                if (wait_for_new_text && editor.IsTextChanged()) {
                    wait_for_new_text = false;
                }
                if (editor_text_cromulent && !wait_for_new_text) {
                    // If we disable value printing, we don't have to export symbols from the executable
                    // to shared libraries.
                    constexpr bool disableValuePrinting = true;
                    auto result = interp.process(editor.GetText(), nullptr, nullptr, disableValuePrinting);
                    if (result != cling::Interpreter::kSuccess) {
                        // Don't spam whatever error there is, wait for new text
                        wait_for_new_text = true;
                    }
                }
            }

            ImGui::EndChild();
            ImGui::PopID();
        }
        ImGui::PopStyleVar(5);
        ImGui::End();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}