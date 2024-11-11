/*
 * ---------------------------------------------------
 * Application.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/02 17:15:13
 * ---------------------------------------------------
 */

#include "Application.hpp"
#include "Graphics/Event.hpp"
#include "Graphics/Platform.hpp"
#include "UtilsCPP/Func.hpp"

namespace GE
{

Application::Application()
    : m_window(gfx::Platform::shared().newWindow(1280, 720)),
      m_renderer(gfx::Platform::shared().newGraphicAPI(m_window))
{
    m_renderer.setOnImGuiRender(utils::Func<void()>(*this, &Application::onImGuiRender));

    gfx::Platform::shared().addEventCallBack(utils::Func<void(gfx::Event&)>(*this, &Application::onEvent), this);
}

void Application::run()
{
    m_running = true;
    while (m_running)
    {
        onUpdate();

        m_renderer.render();

        gfx::Platform::shared().pollEvents();

        for (auto& ctx : m_inputContextPool)
        {
            if (ctx == m_dispatchedInputContext)
                ctx->dispatchInputs();
            else
                ctx->resetInputs();
        }
        m_inputContextPool.clear();
    }
}

void Application::onEvent(gfx::Event& event)
{
    if (event.dispatch<gfx::InputEvent>([&](gfx::InputEvent& inputEvent) {
        for (auto& ctx : m_inputContextPool)
            inputEvent.dispatch(utils::Func<void(gfx::InputEvent&)>(*ctx, &InputContext::onInputEvent));
    })) return;

    if (event.dispatch(utils::Func<void(gfx::WindowResizeEvent&)>(*this, &Application::onWindowResizeEvent)))
        return;

    if (event.dispatch(utils::Func<void(gfx::WindowRequestCloseEvent&)>(*this, &Application::onWindowRequestCloseEvent)))
        return;
}

Application::~Application()
{
    gfx::Platform::shared().clearCallbacks(this);
}

}