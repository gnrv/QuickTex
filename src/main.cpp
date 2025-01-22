#include <iostream>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_internal.h"
#include "implot.h"
#include "implot3d.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "IconsMaterialDesignIcons.h"

#include "Presentation.h"
#include "Slide.h"
#include "TextEditor.h"
#include "imgui_latex.h"
#include "imgui_scale.h"

#include <cmath>

static float f_adjust = 0.0f;

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

#include <string>

#include <chrono>
#include <iostream>
#include <exception>
#include <fmt/format.h>

#include <filesystem>
#include <fstream>

#ifdef USE_CLING
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "cling/Interpreter/Interpreter.h"
#include "cling/Interpreter/Value.h"
#include "cling/MetaProcessor/InputValidator.h"
#endif

#include "system/sys_util.h"

int main(int argc, char **argv) {
    std::filesystem::current_path(getExecutablePath());

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
    interp.AddIncludePath("../external/imgui/implot");
    interp.AddIncludePath("../external/imgui/implot3d");
    interp.AddIncludePath("../external/microtex/imlatex");
    // Pre-include it
    std::vector<std::string> headers = {
        "imgui.h",
        "imgui_latex.h",
        "implot.h",
        "implot3d.h",
        "cmath",
        "cstdio",
        "algorithm",
        "iostream"
    };
    for (const auto& header : headers) {
        auto result = interp.loadHeader(header);
        if (result != cling::Interpreter::kSuccess) {
            std::cerr << "Failed to load header: " << result << std::endl;
            exit(1);
        }
    }

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
    ImVec2 window_size{ 16*window_height*window_size_scale_factor/10, window_height*window_size_scale_factor };
    GLFWwindow* window = glfwCreateWindow(window_size.x,
                                          window_size.y,
                                          "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImPlot3D::CreateContext();
    ImGui::InitLatex();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // NavEnableKeyboard messes with TextEditor
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Instead of disabling assert, we define a throwing IM_ASSERT
    // UPDATE: Nope, the exception was not propagated across the interpreter to us
    io.ConfigErrorRecoveryEnableAssert = false;

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

    // Presentation sizes
    ImFont *fira_sans_big = io.Fonts->AddFontFromFileTTF("../data/fonts/fira/FiraSans-Regular.ttf", 48.0f*dpi_scale);
    ImFont *fira_sans_small = io.Fonts->AddFontFromFileTTF("../data/fonts/fira/FiraSans-Regular.ttf", 32.0f*dpi_scale);

    ImFont *fira_mono = io.Fonts->AddFontFromFileTTF("../data/fonts/fira/FiraMono-Regular.ttf", 16.0f*dpi_scale);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    bool show_demo_window = false;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    //ImVec4 clear_color = ImVec4(0.f, 0.f, 0.f, 1.00f);

    Presentation presentation("../documents/test");
    std::map<std::string, TextEditor> editors;
    editors["setup"].SetText(presentation.setup.text());
    for (int i = 0; i < presentation.slides.size(); i++) {
        editors[fmt::format("slide{}", i)].SetText(presentation.slides[i].text());
    }

    auto ToggleFullscreen = [window_size, window](){
            static int w = window_size.x, h = window_size.y;
        if (glfwGetWindowMonitor(window)) {
            glfwSetWindowMonitor(window, nullptr, 100, 100, w, h, 0);
            glfwSetWindowSize(window, w, h);
        } else {
            // Get the size when windowed
            glfwGetWindowSize(window, &w, &h);
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        };
    };

#ifndef USE_CLING
    presentation.slides[0].function = [](ImVec2 slide_size) {
        #include "test/slide0.cpp"
    };
    presentation.slides[1].function = [](ImVec2 slide_size) {
        #include "test/slide1.cpp"
    };
    presentation.slides[2].function = [](ImVec2 slide_size) {
        #include "test/slide2.cpp"
    };
    presentation.slides[3].function = [](ImVec2 slide_size) {
        #include "test/slide3.cpp"
    };
    presentation.slides[4].function = [](ImVec2 slide_size) {
        #include "test/slide4.cpp"
    };
    presentation.slides[5].function = [](ImVec2 slide_size) {
        #include "test/slide5.cpp"
    };
    presentation.slides[6].function = [](ImVec2 slide_size) {
        #include "test/slide6.cpp"
    };
    presentation.slides[7].function = [](ImVec2 slide_size) {
        #include "test/slide7.cpp"
    };
    presentation.slides[8].function = [](ImVec2 slide_size) {
        #include "test/slide8.cpp"
    };
    presentation.slides[9].function = [](ImVec2 slide_size) {
        #include "test/slide9.cpp"
    };
#endif

    // Main loop
    std::string activate_tab = "slide0";
    std::string active_tab = "slide0";
    std::string exception_what;
    while (!glfwWindowShouldClose(window)) {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // If GLFW reports that key F11 was pressed, toggle fullscreen
        static bool toggle_fullscreen = false;
        if (!toggle_fullscreen && glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS) {
            toggle_fullscreen = true;

            ToggleFullscreen();
        }

        if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_RELEASE) {
            toggle_fullscreen = false;
        }

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
        TextEditor &active_editor = editors[active_tab];
        ImGui::SetNextWindowSize(ImVec2(width/2, height));
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::Begin("Code", 0, flags | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Save")) {
                    try {
                        presentation.getSourceFile(active_tab).save();
                    } catch (std::exception& e) {
                        exception_what = e.what();
                        ImGui::OpenPopup("Exception");
                    }
                }
                if (ImGui::MenuItem("Quit", "Alt-F4"))
                    break;
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
                bool ro = active_editor.IsReadOnly();
                if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
                    active_editor.SetReadOnly(ro);
                ImGui::Separator();

                if (ImGui::MenuItem("Undo", "Ctrl-Z", nullptr, !ro && active_editor.CanUndo()))
                    active_editor.Undo();
                if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && active_editor.CanRedo()))
                    active_editor.Redo();

                ImGui::Separator();

                if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, active_editor.HasSelection()))
                    active_editor.Copy();
                if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && active_editor.HasSelection()))
                    active_editor.Cut();
                if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && active_editor.HasSelection()))
                    active_editor.Delete();
                if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
                    active_editor.Paste();

                ImGui::Separator();

                if (ImGui::MenuItem("Select all", nullptr, nullptr))
                    active_editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(active_editor.GetTotalLines(), 0));

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View")) {
                if (ImGui::MenuItem("Full Screen", "F11"))
                    ToggleFullscreen();
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        if (ImGui::BeginTabBar("MyTabBar")) {
            std::string label_and_id = presentation.setup.path.filename().string() + "###setup";
            ImGuiTabItemFlags flags = 0;
            if (activate_tab == "setup") {
                // The tab will not render it contents until the next lap of the loop.
                flags |= ImGuiTabItemFlags_SetSelected;
            }
            if (presentation.setup.dirty)
                flags |= ImGuiTabItemFlags_UnsavedDocument;
            if (ImGui::BeginTabItem(label_and_id.c_str(), nullptr, flags)) {
                active_tab = "setup";
                if (activate_tab == active_tab)
                    activate_tab.clear();
                ImGui::PushFont(fira_mono);
                auto &editor = editors["setup"];
                editor.Render("TextEditor");
                if (editor.IsTextChanged())
                    try {
                        presentation.getSourceFile("setup").setText(editor.GetText());
                    } catch (std::exception& e) {
                        exception_what = e.what();
                        ImGui::OpenPopup("Exception");
                    }

                ImGui::PopFont();
                ImGui::EndTabItem();
            }
            for (auto &slide : presentation.slides) {
                int i = presentation.indexOf(slide);
                std::string slide_id = fmt::format("slide{}", i);
                std::string label_and_id = slide.path.filename().string() + "###" + slide_id;
                ImGuiTabItemFlags flags = 0;
                if (activate_tab == slide_id) {
                    flags |= ImGuiTabItemFlags_SetSelected;
                }
                if (slide.dirty)
                    flags |= ImGuiTabItemFlags_UnsavedDocument;
                if (ImGui::BeginTabItem(label_and_id.c_str(), nullptr, flags)) {
                    active_tab = slide_id;
                    if (activate_tab == active_tab)
                        activate_tab.clear();
                    ImGui::PushFont(fira_mono);
                    auto &editor = editors[slide_id];
                    editor.Render("TextEditor");
                    if (editor.IsTextChanged())
                        try {
                            presentation.getSourceFile(slide_id).setText(editor.GetText());
                        } catch (std::exception& e) {
                            exception_what = e.what();
                            ImGui::OpenPopup("Exception");
                        }

                    ImGui::PopFont();
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }

        // TextEditor actions
        auto shift = io.KeyShift;
        auto ctrl = io.ConfigMacOSXBehaviors ? io.KeySuper : io.KeyCtrl;
        auto alt = io.ConfigMacOSXBehaviors ? io.KeyCtrl : io.KeyAlt;

        if (ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGuiKey_S)) {
            try {
                presentation.getSourceFile(active_tab).save();
            } catch (std::exception& e) {
                exception_what = e.what();
                ImGui::OpenPopup("Exception");
            }
        }

        // TextEditor dialogs
        if (ImGui::BeginPopupModal("Save Failed", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("%s", exception_what.c_str());
            if (ImGui::Button("OK"))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
        ImGui::End();

        // 2. Presentation window
        ImGui::SetNextWindowSize(ImVec2(width/2, height));
        ImGui::SetNextWindowPos(ImVec2(width/2, 0));
        // Get rid of horizontal padding
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
        // Transparent scrollbar bg
        // Doesn't work, the scrollbar always affects clip rect and layout
        // if we want an overlaid scroll bar, we need to draw it ourselves
        // So disable the built in scrollbar completely
        flags |= ImGuiWindowFlags_NoScrollbar;
        // Let the presentation area contain 10 placeholder slides
        // of aspect ratio 16:10
        ImGui::Begin("Presentation", 0, flags);

        // Slides are designed for 1080p, 16:10 aspect ratio
        // TODO: Once ImGui::SetScale is better implemented across imgui and the glfw backend, we can
        //       use a fixed slide_size of 1728x1080 and use ImGui::PushScale() to accomplish this.
        ImVec2 slide_size{ width/2, width/2*10/16 };
        float slide_scale = slide_size.y / 1080.f;
        // Watch out, my PushScale implementation multiplies onto the current DPI scale
        // so we need to divide by that here.
        slide_scale /= dpi_scale;

        // Before all the slides, the "setup" placeholder
        // Calculate the height of one row of ImGui::Text
        float text_height = ImGui::GetTextLineHeightWithSpacing();
        ImGui::BeginChild("Setup", ImVec2(slide_size.x, slide_size.y/2 - text_height), false);
        ImGui::EndChild();
        for (int i = 0; i < 10; i++) {
            ImGui::PushID(i);
            ImGui::Text("Slide %d", i);
            ImGui::SameLine();
            ImGui::SetCursorPosX(slide_size.x - ImGui::GetStyle().FramePadding.x * 2 -
                                 ImGui::CalcTextSize(ICON_MDI_REFRESH).x -
                                 ImGui::CalcTextSize(ICON_MDI_PENCIL).x);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            std::string slide_id = fmt::format("slide{}", i);
            if (ImGui::Button(ICON_MDI_PENCIL))
                activate_tab = slide_id;
            ImGui::SameLine();
            ImGui::Button(ICON_MDI_REFRESH);
            ImGui::PopStyleColor(1);

            auto top_left = ImGui::GetCursorScreenPos();
            ImGui::BeginChild("Slide", slide_size, false);
            ImGui::PushFont(fira_sans_big);
            ImGui::PushScale(slide_scale);

            // The problem here is that the drawlist uses window coordinates.
            // We need to convert the coordinates to window coordinates.
            // We can do this by using the cursor position.
            ImGui::GetWindowDrawList()->AddRect(top_left, top_left + slide_size, IM_COL32(255, 255, 255, 127));
            SourceFile &slide_src = presentation.slides[i];
#ifdef USE_CLING
            if (!slide_src.validated) {
                try {
                    cling::InputValidator validator;
                    auto result = validator.validate(slide_src.text());
                    slide_src.setValidated(result == cling::InputValidator::kComplete);
                } catch (std::exception& e) {
                    exception_what = e.what();
                    ImGui::OpenPopup("Exception");
                }
            }

            if (slide_src.validated && !slide_src.compiled && !slide_src.syntax_error) {
                // If we disable value printing, we don't have to export symbols from the executable
                // to shared libraries.
                cling::Value V;
                // if (slide_src.last_transaction)
                //     interp.unload(*slide_src.last_transaction);
                slide_src.last_transaction = nullptr;
                auto result = interp.process("void (*update)(ImVec2 slide_size) = [](ImVec2 slide_size){" + slide_src.text() + "}; update", &V, &slide_src.last_transaction, true /* disableValuePrinting */);
                slide_src.compiled = true;
                if (result != cling::Interpreter::kSuccess) {
                    slide_src.last_transaction = nullptr; // Should be done by cling, but just in case
                    slide_src.syntax_error = true;
                } else {
                    slide_src.syntax_error = false;
                }
                // The value in lastV should be a function that we call to re-render the slide
                if (V.isValid()) {
                    slide_src.function = reinterpret_cast<void (*)(ImVec2)>(V.getPtr());
                } else {
                    slide_src.function = nullptr;
                }
            }
#endif
            ImGuiErrorRecoveryState state;
            ImGui::ErrorRecoveryStoreState(&state);
            try {
                if (slide_src.function) slide_src.function(slide_size);
            } catch (std::exception& e) {
                ImGui::ErrorRecoveryTryToRecoverState(&state);
                std::cerr << "Caught exception: " << e.what() << std::endl;
            }
            ImGui::PopFont();
            ImGui::PopScale();
            ImGui::EndChild();
            ImGui::Text(""); // Just add some space for symmetry
            ImGui::PopID();
        }

        // Add a spacer to allow the last slide to be scrolled into view and centered vertically
        ImGui::BeginChild("Final Spacer", ImVec2(slide_size.x, slide_size.y/2 - text_height), false);
        ImGui::EndChild();

        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(5);

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f_adjust, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
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
    ImPlot3D::DestroyContext();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}