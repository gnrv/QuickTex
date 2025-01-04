# Require minimum standard version of CMake
cmake_minimum_required (VERSION 3.14)

# Set C++ 17 compiler flags
set(CMAKE_CXX_STANDARD 17)

# Set project name
project (Imgui LANGUAGES CXX)

# Set the source files to a list
set(
    source_list
	"imgui/imconfig.h"
    "imgui/imgui.cpp"
	"imgui/imgui.h"
	"imgui/imgui_demo.cpp"
	"imgui/imgui_draw.cpp"
	"imgui/misc/cpp/imgui_stdlib.h"
	"imgui/misc/cpp/imgui_stdlib.cpp"
#	"imgui/misc/freetype/imgui_freetype.cpp"
	"imgui/imgui_internal.h"
    "imgui/imgui_tables.cpp"
	"imgui/imgui_widgets.cpp"
	"imgui/imstb_rectpack.h"
	"imgui/imstb_textedit.h"
	"imgui/imstb_truetype.h"
	"imgui/backends/imgui_impl_glfw.cpp"
	"imgui/backends/imgui_impl_opengl3.cpp"
)


# Create executable file as project name
add_library(Imgui ${source_list})

# If using IMGUI_ENABLE_STB_TRUETYPE
# find_package(Freetype REQUIRED)
# target_link_libraries(Imgui Freetype::Freetype)
target_compile_definitions(Imgui PUBLIC IMGUI_USE_WCHAR32)
target_include_directories(Imgui PUBLIC "${PROJECT_SOURCE_DIR}/imgui")
