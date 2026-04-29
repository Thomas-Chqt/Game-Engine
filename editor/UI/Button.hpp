/*
 * ---------------------------------------------------
 * Button.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef BUTTON_HPP
#define BUTTON_HPP

#include "UI/ViewBase.hpp"

#include <imgui.h>

#include <string>
#include <utility>

namespace GE_Editor::UI
{

template <class... Args>
class Button : public ViewBase<Button<Args...>>
{
public:
    Button(std::string text)
        : m_text(std::move(text))
    {
    }

    void render() const
    {
        this->renderLambda([&](){
            ImGui::Button(m_text.c_str(), m_size);
        });
    }

    inline Button& size(float w, float h) { return m_size = {w,h}, *this; }

private:
    std::string m_text;
    ImVec2 m_size = {0, 0};
};

}

#endif // BUTTON_HPP
