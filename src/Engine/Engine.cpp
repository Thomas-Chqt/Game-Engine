/*
 * ---------------------------------------------------
 * Engine.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/31 12:53:04
 * ---------------------------------------------------
 */

#include "Engine/Engine.hpp"
#include "Graphics/Platform.hpp"
#include <utility>

namespace GE
{

Engine::Engine(const utils::SharedPtr<gfx::Window>& win)
    : m_window(win), m_renderer(gfx::Platform::shared().newGraphicAPI(m_window))
{
}

void Engine::runApplication(utils::UniquePtr<Application>&& app)
{
    m_application = std::move(app);

    m_application->onSetup();

    while (m_application->shouldTerminate() == false)
    {
        m_application->onUpdate();
    }
}

}