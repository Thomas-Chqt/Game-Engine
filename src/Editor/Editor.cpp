/*
 * ---------------------------------------------------
 * Editor.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/31 11:01:01
 * ---------------------------------------------------
 */

#include "Editor/Editor.hpp"
#include "Graphics/Event.hpp"
#include "Graphics/Platform.hpp"
#include "UtilsCPP/Func.hpp"

namespace GE
{

Editor::Editor(int argc, char* argv[])
    : m_window(gfx::Platform::shared().newWindow(1280, 720)),
      m_graphicAPI(gfx::Platform::shared().newGraphicAPI(m_window)),
      m_renderer(*m_graphicAPI)
{
    m_window->addEventCallBack(utils::Func<void(gfx::Event&)>(*this, &Editor::onEvent), this);
    m_graphicAPI->initImgui();
    m_renderer.setOnImGuiRender(utils::Func<void()>(*this, &Editor::onImGuiRender));
}

Editor::~Editor()
{
    m_window->clearCallbacks(this);
}

void Editor::onEvent(gfx::Event&)
{

}

void Editor::onImGuiRender()
{

}

}