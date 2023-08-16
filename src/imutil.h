#pragma once 

#include "imgui_stdlib.h"
#include <tempo.h>

bool operator==(const ImVec4& left, const ImVec4& right) {
    return left.x == right.x && left.y == right.y && left.z == right.z && left.w == right.w;
}
bool operator!= (const ImVec4& left, const ImVec4& right) {
    return !(left == right);
}