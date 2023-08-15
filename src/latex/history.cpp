#include "history.h"

#include <chrono>
#include <toml.hpp>
#include "core/colors.h"

void History::save_to_file() {
    std::fstream fs("data/history.toml", std::ios::in | std::ios::out | std::ios::trunc);
    auto file = toml::parse(fs);

    for (auto& pair : m_history) {
        file["history"][std::to_string(pair.first)] = {
            {"latex", pair.second.latex},
            {"aspect_ratio", pair.second.aspect_ratio}
        };
    }
    for (auto& pair : m_bookmarks) {
        file["bookmarks"][std::to_string(pair.first)] = {
            {"latex", pair.second.latex},
            {"aspect_ratio", pair.second.aspect_ratio},
            {"bookmark_name", pair.second.bookmark_name}
        };
    }
    std::ofstream("data/history.toml") << file;
}

void History::load() {
    if (m_is_loaded)
        return;
    m_is_loaded = true;
    std::fstream fs("data/history.toml", std::ios::in);
    if (!fs.is_open()) {
        std::fstream out("data/history.toml", std::ios::out | std::ios::trunc);
    }
    auto file = toml::parse("data/history.toml");
    auto history = toml::find_or<toml::table>(file, "history", toml::table());
    for (auto& pair : history) {
        auto& value = pair.second;
        LatexHistory history_point;
        history_point.latex = toml::find_or<std::string>(value, "latex", "");
        history_point.aspect_ratio = toml::find_or<float>(value, "aspect_ratio", 1.f);
        history_point.timepoint = std::stoull(pair.first);
        history_point.is_inline = toml::find_or<bool>(value, "is_inline", false);
        m_history[std::stoull(pair.first)] = history_point;
    }

    auto bookmarks = toml::find_or<toml::table>(file, "bookmarks", toml::table());
    for (auto& pair : bookmarks) {
        auto& value = pair.second;
        LatexHistory bookmark;
        bookmark.latex = toml::find_or<std::string>(value, "latex", "");
        bookmark.aspect_ratio = toml::find_or<float>(value, "aspect_ratio", 1.f);
        bookmark.bookmark_name = toml::find_or<std::string>(value, "bookmark_name", "");
        bookmark.timepoint = std::stoull(pair.first);
        bookmark.is_inline = toml::find_or<bool>(value, "is_inline", false);
        m_bookmarks[std::stoull(pair.first)] = bookmark;
    }
}

void History::saveToHistory(LatexHistory history_point) {
    load();
    uint64_t timepoint = std::chrono::system_clock::now().time_since_epoch().count();
    history_point.timepoint = timepoint;
    m_history[timepoint] = history_point;
    save_to_file();
}
void History::saveBookmark(LatexHistory bookmark) {
    load();
    uint64_t timepoint = std::chrono::system_clock::now().time_since_epoch().count();
    bookmark.timepoint = timepoint;
    m_bookmarks[timepoint] = bookmark;
    save_to_file();
}

std::vector<LatexHistory> History::getHistory() {
    load();
    std::vector<LatexHistory> out;
    for (auto it = m_history.rbegin(); it != m_history.rend(); ++it) {
        out.push_back(it->second);
    }
    return out;
}
std::vector<LatexHistory> History::getBookmarks() {
    load();
    std::vector<LatexHistory> out;
    for (auto it = m_bookmarks.rbegin(); it != m_bookmarks.rend(); ++it) {
        out.push_back(it->second);
    }
    return out;
}

void History::clean() {
    for (auto& timepoint : m_to_erase) {
        m_history.erase(timepoint);
        m_bookmarks.erase(timepoint);
    }
    if (!m_to_erase.empty())
        save_to_file();
    m_to_erase.clear();
}
bool History::must_retrieve_latex(LatexHistory& history_point) {
    if (m_to_retrieve == 0)
        return false;
    if (m_history.find(m_to_retrieve) != m_history.end()) {
        history_point = m_history[m_to_retrieve];
        m_to_retrieve = 0;
        return true;
    }
    return false;
}

void History::show_history_point(const LatexHistory& history_point, const Rect& boundaries) {
    ImGui::PushID(history_point.timepoint);
    if (ImGui::Button("X")) {
        m_to_erase.insert(history_point.timepoint);
    }
    ImGui::SameLine();
    if (m_history_images.find(history_point.timepoint) == m_history_images.end()) {
        m_history_images[history_point.timepoint] = std::make_unique<Latex::LatexImage>(history_point.latex, 50.f, 1.2f);
    }
    // Show image
    auto image = m_history_images[history_point.timepoint]->getImage();
    float width = image->width();
    float height = image->height();
    float aspect_ratio = width / height;
    float max_width = boundaries.w - 50.f;
    if (width > max_width) {
        width = max_width;
        height = width / aspect_ratio;
    }
    auto cursor = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(cursor.x, cursor.y), ImVec2(cursor.x + width, cursor.y + height), Colors::white, 0.f, 0);
    ImGui::Image((void*)image->texture(), ImVec2(width, height), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1.f, 1.f, 1.f, 1.f), ImGui::GetStyleColorVec4(ImGuiCol_FrameBgHovered));
    if (ImGui::IsItemClicked()) {
        m_to_retrieve = history_point.timepoint;
        m_open = false;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }
    ImGui::PopID();
}

void History::draw() {
    if (m_open)
        ImGui::OpenPopup("History");
    load();
    if (ImGui::BeginPopupModal("History", &m_open)) {
        Rect boundaries;
        boundaries.x = ImGui::GetWindowPos().x;
        boundaries.y = ImGui::GetWindowPos().y;
        boundaries.w = ImGui::GetWindowSize().x;
        boundaries.h = ImGui::GetWindowSize().y;

        for (auto it = m_history.rbegin(); it != m_history.rend(); ++it) {
            show_history_point(it->second, boundaries);
        }
        ImGui::EndPopup();
    }
    clean();
}