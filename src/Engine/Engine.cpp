/*
 * ---------------------------------------------------
 * Engine.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/31 12:53:04
 * ---------------------------------------------------
 */

#include "Engine/Engine.hpp"
#include "Game-Engine/Game.hpp"
#include "Game/StarterContent.hpp"
#include "Graphics/Event.hpp"
#include "Graphics/KeyCodes.hpp"
#include "Graphics/Platform.hpp"
#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

void openProject(const utils::String& filepath)
{
}

void Engine::run()
{
    m_running = true;
    while (m_running)
    {
        gfx::Platform::shared().pollEvents();
    }
}

Engine::~Engine()
{
    m_renderer.setOnImGuiRender(utils::Func<void ()>());
    gfx::Platform::shared().clearCallbacks(this);
}

Engine::Engine()
    : m_window(gfx::Platform::shared().newWindow(1280, 720)),
      m_renderer(gfx::Platform::shared().newGraphicAPI(m_window))
{
    m_game = utils::makeUnique<StarterContent>().staticCast<Game>();

    gfx::Platform::shared().addEventCallBack(utils::Func<void(gfx::Event&)>(*this, &Engine::onEvent));
    m_renderer.setOnImGuiRender(utils::Func<void()>(*this, &Engine::onImGuiRender));
}

void Engine::onEvent(gfx::Event& event)
{
    event.dispatch<gfx::KeyDownEvent>([&](gfx::KeyDownEvent& keyDownEvent) {
        if (keyDownEvent.keyCode() == ESC_KEY)
            m_running = false;
    });
    event.dispatch<gfx::WindowRequestCloseEvent>([&](gfx::WindowRequestCloseEvent& windowRequestCloseEvent) {
        m_running = false;
    });
}

void Engine::onImGuiRender()
{
}

}