# Set C++ 17 compiler flags
if(MSVC)
    cmake_minimum_required(VERSION 3.21)
else()
    cmake_minimum_required(VERSION 3.17)
endif()

set(CMAKE_CXX_STANDARD 20)

# Set project name
project(quicktex)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(INSTALL_DIR "0.1")

# ---- Cairo ----
find_package(Cairo)
include_directories(${CAIRO_INCLUDE_DIRS})

# ---- MicroTex ----
add_subdirectory(external/microtex)
include_directories(external/microtex)

# ---- Tempo app ----
# ImGui config
add_subdirectory(external/TempoApp)
include_directories(external/TempoApp/include)
include_directories(external/TempoApp/external/imgui/imgui)

# ----- Rapidfuzz ------
add_subdirectory(external/rapidfuzz-cpp)
include_directories(external/rapidfuzz-cpp/include)

# -----
# Fonts
# -----
include_directories(external/IconFontCppHeaders)

# ---------------
# STB image write
# ---------------
include_directories(external/stb_image)

# ----
# Clip
# ----
set(CLIP_EXAMPLES OFF CACHE BOOL "Compile clip examples")
set(CLIP_TESTS OFF CACHE BOOL "Compile clip tests")
add_subdirectory(external/clip)

# Sources of program
include_directories(src)
file(GLOB source_list RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    src/system/*.cpp
    src/search/*.cpp
    src/chars/drawable_char.cpp
    src/chars/*.cpp
    src/latex/*.cpp
    src/fonts/*.cpp
    src/core/*.cpp
    src/window/*.cpp
)

# Making a library without the main, for the tests and the program
add_library(${PROJECT_NAME}_lib ${source_list})

# Remove console on Windows
# if(MSVC)
# # /ENTRY:mainCRTStartup keeps the same "main" function instead of requiring "WinMain"
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
# else()
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")

# set(SUBSYSTEM_LINKER_OPTIONS "-mwindows")
# target_link_options(${PROJECT_NAME} PRIVATE ${SUBSYSTEM_LINKER_OPTIONS})
# endif()

# Main program executable
add_executable(${PROJECT_NAME} src/main.cpp ${APP_ICON_RESOURCE_WINDOWS})

set(LIB_LINK microtex-imgui ${CAIRO_LIBRARIES} clip rapidfuzz::rapidfuzz)
target_link_libraries(${PROJECT_NAME}_lib ${LIB_LINK})
target_include_directories(${PROJECT_NAME}_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/external/clip)
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_lib)

# Copy the data (such as fonts) in build directory
add_custom_command(TARGET quicktex PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data/ $<TARGET_FILE_DIR:quicktex>/data)

if(MSVC)
    # CMake 3.21 is for this functionnality / Copy the dlls
    add_custom_command(TARGET quicktex POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:quicktex> $<TARGET_FILE_DIR:quicktex>
        COMMAND_EXPAND_LISTS
    )
endif()

# Set compiler options
if(MSVC)
    # /ENTRY:mainCRTStartup keeps the same "main" function instead of requiring "WinMain"
    target_compile_options(${PROJECT_NAME} PRIVATE /W0)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()

if(WIN32)
    # if config is debug set the variable to "/SUBSYSTEM:CONSOLE," else "/SUBSYSTEM:WINDOWS", append "/ENTRY:mainCRTStartup>" in any case
    set(SUBSYSTEM_LINKER_OPTIONS "$<IF:$<CONFIG:Debug>,/SUBSYSTEM:CONSOLE,/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup>")
else()
    # if config is not debug set the variable to "-mwindows" else nothing
    set(SUBSYSTEM_LINKER_OPTIONS "$<IF:$<NOT:$<CONFIG:Debug>>,-mwindows,>")
endif()

target_link_options(${PROJECT_NAME} PRIVATE ${SUBSYSTEM_LINKER_OPTIONS})