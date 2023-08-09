#pragma once

#include <memory>
#include <tempo.h>
#include "fonts/fonts.h"

struct UIState {
protected:
    UIState() {

    }
public:
    bool read_only = false;
    long long int imID = 100000000;
    float scaling = 1.f;

    UIState(UIState const&) = delete;
    void operator=(UIState const&) = delete;

    static UIState& getInstance() {
        static UIState instance;
        return instance;
    }

    // Fonts
    Fonts::FontManager font_manager;

    // Languages
};
typedef std::shared_ptr<UIState> UIState_ptr;