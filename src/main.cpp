#include <iostream>
#define IMGUI_USE_WCHAR32
#include <tempo.h>
#include <string>

#include <chrono>
#include <iostream>
#include <exception>

#include "latex/latex.h"

#include "window/main_window.h"

#include <chrono>
using namespace std::chrono;

int main() {
    std::string err = Latex::init();
    Tempo::Config config;
    config.app_name = "QuickTex";
    config.app_title = "QuickTex - Quickly create math equations";
    config.imgui_config_flags = ImGuiConfigFlags_DockingEnable;
    config.poll_or_wait = Tempo::Config::POLL;
    config.default_window_width = 720;
    config.default_window_height = 600;

    MainApp* app = new MainApp(err);
    Tempo::Run(app, config);

    return 0;
}