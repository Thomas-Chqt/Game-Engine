/*
 * ---------------------------------------------------
 * EngineIntern.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/06/20 17:15:45
 * ---------------------------------------------------
 */

#include "Game-Engine/AssetManager.hpp"
#include "Graphics/Event.hpp"
#include "Graphics/Platform.hpp"
#include "Renderer/GPURessourceManager.hpp"
#include "Renderer/Renderer.hpp"
#include "UtilsCPP/Func.hpp"
#include "Engine/EngineIntern.hpp"
#include <utility>

namespace GE
{

void EngineIntern::runGame(utils::UniquePtr<Game>&& gameToRun)
{
    m_runningGame = std::move(gameToRun);

    m_gameWindow = gfx::Platform::shared().newWindow(800, 600);

    Renderer::shared().setWindow(m_gameWindow);

    m_runningGame->onSetup();
    while (1)
    {
        gfx::Platform::shared().pollEvents();
        if (m_runningGame == nullptr)
            break;

        m_runningGame->activeScene().onUpdate();
    }
}

EngineIntern::~EngineIntern()
{
    AssetManager::terminate();
    GPURessourceManager::terminate();
    Renderer::terminate();

    gfx::Platform::shared().clearCallbacks(this);
    gfx::Platform::terminate();
}

EngineIntern::EngineIntern()
{
    gfx::Platform::init();
    gfx::Platform::shared().addEventCallBack(utils::Func<void(gfx::Event&)>(*this, &EngineIntern::onEvent), this);

    Renderer::init();
    GPURessourceManager::init();
    AssetManager::init();
}

void EngineIntern::onEvent(gfx::Event& event)
{
    if (event.dispatch<gfx::WindowRequestCloseEvent>([&](gfx::WindowRequestCloseEvent& event) {
        if (m_runningGame)
            m_runningGame->onWindowRequestCloseEvent();
    })) return;
    
    if (event.dispatch<gfx::KeyDownEvent>([&](gfx::KeyDownEvent& event) {
        if (m_runningGame)
            m_runningGame->onKeyDownEvent(event.keyCode(), event.isRepeat());
        if (event.isRepeat() == false)
            m_pressedKeys.insert(event.keyCode());
    })) return;
    
    if (event.dispatch<gfx::KeyUpEvent>([&](gfx::KeyUpEvent& event) {
        m_pressedKeys.remove(m_pressedKeys.find(event.keyCode()));
    })) return;
}

}