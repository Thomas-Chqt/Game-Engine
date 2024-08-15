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
    gfx::Platform::init();

    utils::SharedPtr<gfx::Window> window = gfx::Platform::shared().newWindow(800, 600);
    utils::SharedPtr<gfx::GraphicAPI> graphicAPI = gfx::Platform::shared().newGraphicAPI(window);
    graphicAPI->initImgui();

    GPURessourceManager::init(graphicAPI);

    EngineIntern::init(std::move(window));
}

void Engine::terminate()
{
    s_sharedInstance.clear();
    GPURessourceManager::terminate();
    gfx::Platform::terminate();
}

void EngineIntern::init(utils::SharedPtr<gfx::Window>&& win)
{
    Engine::s_sharedInstance = utils::UniquePtr<EngineIntern>(new EngineIntern(std::move(win))).staticCast<Engine>();
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

        scriptSystem();

        m_renderer.beginScene(getActiveCameraSystem(), *m_mainWindow);
        {
            addLightsSystem();
            addRenderableSystem();
        }
        m_renderer.endScene();

        m_renderer.render();
    }
}

EngineIntern::~EngineIntern()
{
    AssetManager::terminate();

    gfx::Platform::shared().clearCallbacks(this);    
}

EngineIntern::EngineIntern(utils::SharedPtr<gfx::Window>&& window)
    : m_mainWindow(std::move(window))
{
    m_renderer.setOnImGuiRender(utils::Func<void()>(*this, &EngineIntern::onImGuiRender));

    gfx::Platform::shared().addEventCallBack(utils::Func<void(gfx::Event&)>(*this, &EngineIntern::onEvent), this);
 
    AssetManager::init();
}

void EngineIntern::onEvent(gfx::Event& event)
{
    event.dispatch<gfx::KeyDownEvent>([&](gfx::KeyDownEvent& event) {
        if (event.isRepeat() == false)
            m_pressedKeys.insert(event.keyCode());
    });
    
    event.dispatch<gfx::KeyUpEvent>([&](gfx::KeyUpEvent& event) {
        m_pressedKeys.remove(m_pressedKeys.find(event.keyCode()));
    });

    assert(m_game);
    m_game->onEvent(event);
}

}