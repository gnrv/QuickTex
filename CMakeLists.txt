# Set C++ 17 compiler flags
if(MSVC)
    cmake_minimum_required(VERSION 3.21)
else()
    cmake_minimum_required(VERSION 3.17)
endif()

set(CMAKE_CXX_STANDARD 20)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

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

# ---- ImGui ----
# ImGui config
add_subdirectory(external/imgui)
include_directories(external/imgui/imgui)

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

# ----
# GLAD
# ----
#add_subdirectory(external/glad)

# ----
# GLFW
# ----
find_package(glfw3 REQUIRED)

# -----
# CLING
# -----
set(LLVM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/root-project/cling-build/lib/cmake/llvm")
find_package(LLVM REQUIRED CONFIG NO_DEFAULT_PATH)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

set(Clang_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/root-project/cling-build/lib/cmake/clang")
find_package(Clang REQUIRED CONFIG NO_DEFAULT_PATH)
message(STATUS "Found Clang ${Clang_PACKAGE_VERSION}")
message(STATUS "Using ClangConfig.cmake in: ${Clang_DIR}")

set(Cling_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/root-project/cling-build/tools/cling/lib/cmake/cling")
find_package(Cling REQUIRED CONFIG NO_DEFAULT_PATH)
message(STATUS "Found Cling ${Cling_PACKAGE_VERSION}")
message(STATUS "Using ClingConfig.cmake in: ${Cling_DIR}")


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
add_library(${PROJECT_NAME}_lib
    ${source_list}
)

# Main program executable
set(LIBS
clingInterpreter
)
add_executable(${PROJECT_NAME}
src/main.cpp
${CMAKE_CURRENT_SOURCE_DIR}/external/ImGuiColorTextEdit/TextEditor.cpp
${APP_ICON_RESOURCE_WINDOWS}
)
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBS})

# target_link_libraries(${PROJECT_NAME} PRIVATE
# ${CMAKE_CURRENT_SOURCE_DIR}/external/root-project/cling-build/lib/libclingInterpreter.a
# ${CMAKE_CURRENT_SOURCE_DIR}/external/root-project/cling-build/lib/libclingMetaProcessor.a
# ${CMAKE_CURRENT_SOURCE_DIR}/external/root-project/cling-build/lib/libclingUtils.a
# )

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/ImGuiColorTextEdit)
target_include_directories(${PROJECT_NAME} PUBLIC ${LLVM_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${CLANG_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${CLING_INCLUDE_DIRS})

set(LIB_LINK microtex-imgui ${CAIRO_LIBRARIES} clip rapidfuzz::rapidfuzz Imgui GL glfw)
#target_link_libraries(${PROJECT_NAME}_lib PUBLIC someObjs)
target_link_libraries(${PROJECT_NAME}_lib PUBLIC ${LIB_LINK})
target_include_directories(${PROJECT_NAME}_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/external/clip)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_lib)

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

# Remove console on Windows
if(WIN32)
    set_target_properties(${PROJECT_NAME}
        PROPERTIES
        LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE"
        LINK_FLAGS_RELEASE "/SUBSYSTEM:windows /ENTRY:mainCRTStartup"
        LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:windows /ENTRY:mainCRTStartup"
        LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:windows /ENTRY:mainCRTStartup"
    )
endif()

target_link_options(${PROJECT_NAME} PRIVATE ${SUBSYSTEM_LINKER_OPTIONS})

# if(LINUX)
#     install(TARGETS ${PROJECT_NAME}
#         RUNTIME DESTINATION $ENV{HOME}
#         LIBRARY DESTINATION $ENV{HOME}
#         DESTINATION $ENV{HOME}
#     )

#     SET(CPACK_GENERATOR "DEB")
#     SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "KK")
# endif()