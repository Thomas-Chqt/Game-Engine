/*
 * ---------------------------------------------------
 * Engine.cpp
 *
 * Author: Thomas Choquet <thomas.publique@icloud.com>
 * Date: 2024/06/20 17:15:45
 * ---------------------------------------------------
 */

#include "Engine/Engine.hpp"
#include "Graphics/Event.hpp"
#include "Graphics/Platform.hpp"
#include "Game-Engine/Engine.hpp"
#include "UtilsCPP/Func.hpp"

namespace GE
{

utils::UniquePtr<EngineSingleton> EngineSingleton::s_sharedInstance;

void EngineSingleton::onEvent(gfx::Event& event)
{
    event.dispatch<gfx::WindowRequestCloseEvent>([this](gfx::WindowRequestCloseEvent& event)
    {
        m_game->onWindowCloseEvent();
    });
}

void EngineSingleton::runGame(utils::UniquePtr<Game>&& game)
{
    m_game = std::move(game);

    m_gameWindow = gfx::Platform::shared().newDefaultWindow(800, 600);
    m_gameWindow->addEventCallBack(utils::Func<void(gfx::Event&)>(*this, &EngineSingleton::onEvent));

    m_gameAPI = gfx::Platform::shared().newDefaultGraphicAPI(m_gameWindow);

    m_running = true;
    while (m_running)
    {
        gfx::Platform::shared().pollEvents();
        m_gameAPI->beginFrame();
        m_gameAPI->beginOnScreenRenderPass();
        m_gameAPI->endOnScreenRenderPass();
        m_gameAPI->endFrame();
    }
}

void Engine::terminateGame()
{
    EngineSingleton::shared().terminateGame();
}

}