/*
 * ---------------------------------------------------
 * Text.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef TEXT_HPP
#define TEXT_HPP

#include "imgui.h"

#include <string>

namespace GE_Editor::UI
{

template <class... Args>
class Text
{
public:
    Text(std::string text)
        : m_text(std::move(text))
    {
    }

    void render() const
    {
        if (m_size.x == 0 || m_size.y == 0)
        {
            ImGui::TextUnformatted(m_text.c_str());
        }
        else
        {
            ImVec2 textMin = ImGui::GetCursorScreenPos();
            ImVec4 clipRect(textMin.x, textMin.y, textMin.x + m_size.x, textMin.y + m_size.y);
            ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(), textMin, ImGui::GetColorU32(ImGuiCol_Text), m_text.c_str(), nullptr, 0.0f, &clipRect);
            ImGui::Dummy(m_size);
        }
    }

    inline Text& size(float w, float h) { return m_size = {w,h}, *this; }

private:
    std::string m_text;
    ImVec2 m_size = {0, 0};
};

}

#endif // TEXT_HPP
