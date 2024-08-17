/*
 * ---------------------------------------------------
 * EngineIntern.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/06/20 17:15:45
 * ---------------------------------------------------
 */

#include "Engine/EngineIntern.hpp"
#include "Game-Engine/AssetManager.hpp"
#include "Game-Engine/Engine.hpp"
#include "Game-Engine/Game.hpp"
#include "Graphics/Event.hpp"
#include "Graphics/GraphicAPI.hpp"
#include "Graphics/Platform.hpp"
#include "GPURessourceManager.hpp"
#include "InputManager.hpp"
#include "Renderer/Renderer.hpp"
#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/UniquePtr.hpp"
#include <cassert>
#include <cstring>
#include <utility>

namespace GE
{

void Engine::init()
{
    EngineIntern::init();
}

void EngineIntern::runGame(utils::UniquePtr<Game>&& game)
{
    m_game = std::move(game);

    m_game->onSetup();

    m_running = true;
    while (m_running)
    {
        gfx::Platform::shared().pollEvents();

        m_game->onUpdate();

        Renderer& render = Renderer::shared();

        scriptSystem();

        render.beginScene(getActiveCameraSystem(), *m_mainWindow);
        {
            addLightsSystem();
            addRenderableSystem();
        }
        render.endScene();

        render.render();
    }
}

EngineIntern::~EngineIntern()
{
    AssetManager::terminate();
    InputManager::terminate();

    Renderer::terminate();

    GPURessourceManager::terminate();

    gfx::Platform::shared().clearCallbacks(this);
    gfx::Platform::terminate();
}

EngineIntern::EngineIntern()
{
    gfx::Platform::init();
    gfx::Platform::shared().addEventCallBack(utils::Func<void(gfx::Event&)>(*this, &EngineIntern::onEvent), this);

    m_mainWindow = gfx::Platform::shared().newWindow(800, 600);
    utils::SharedPtr<gfx::GraphicAPI> graphicAPI = gfx::Platform::shared().newGraphicAPI(m_mainWindow);
    graphicAPI->initImgui();
    GPURessourceManager::init(graphicAPI);

    Renderer::init();
    Renderer::shared().setOnImGuiRender(utils::Func<void()>(*this, &EngineIntern::onImGuiRender));

    InputManager::init();
    AssetManager::init();
}

void EngineIntern::onEvent(gfx::Event& event)
{
    assert(m_game);
    m_game->onEvent(event);
}

}