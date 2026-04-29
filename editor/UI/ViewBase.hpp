/*
 * ---------------------------------------------------
 * ViewBase.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef VIEWBASE_HPP
#define VIEWBASE_HPP

#include <functional>

#include <imgui.h>
#include <string>

namespace GE_Editor::UI
{

template<typename Derived>
class ViewBase
{
public:
    Derived& disabled(bool d)
    {
        m_disabled = d;
        return static_cast<Derived&>(*this);
    }

    Derived& onClick(std::function<void()> f)
    {
        m_onClick = std::move(f);
        return static_cast<Derived&>(*this);
    }

    Derived& onDoubleClick(std::function<void()> f)
    {
        m_onDoubleClick = std::move(f);
        return static_cast<Derived&>(*this);
    }

    Derived& dragDropSource(std::function<void()> fn)
    {
        m_dragDropSourceCallback = std::move(fn);
        return static_cast<Derived&>(*this);
    }

protected:
    void renderLambda(const std::function<void()> lambda) const
    {
        ImGui::BeginDisabled(m_disabled);
        {
            lambda();
            if (m_dragDropSourceCallback && ImGui::BeginDragDropSource())
            {
                m_dragDropSourceCallback();
                ImGui::EndDragDropSource();
            }
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && m_onClick)
                m_onClick();
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && m_onDoubleClick)
                m_onDoubleClick();
        }
        ImGui::EndDisabled();
    }

private:
    bool m_disabled = false;

    std::function<void()> m_onClick;

    std::function<void()> m_onDoubleClick;

    std::function<void()> m_dragDropSourceCallback;
};

} // namespace GE_Editor::UI

#endif
