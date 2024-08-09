/*
 * ---------------------------------------------------
 * EngineIntern.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/06/20 17:15:45
 * ---------------------------------------------------
 */

#include "Graphics/Event.hpp"
#include "Graphics/Platform.hpp"
#include "UtilsCPP/Func.hpp"
#include "Engine/EngineIntern.hpp"
#include <utility>

namespace GE
{

void EngineIntern::runGame(utils::UniquePtr<Game>&& gameToRun)
{
    m_runningGame = std::move(gameToRun);

    m_gameWindow = gfx::Platform::shared().newWindow(800, 600);

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
    gfx::Platform::shared().clearCallbacks(this);
    gfx::Platform::terminate();
}

EngineIntern::EngineIntern()
{
    gfx::Platform::init();
    gfx::Platform::shared().addEventCallBack(utils::Func<void(gfx::Event&)>(*this, &EngineIntern::onEvent), this);
}

void EngineIntern::onEvent(gfx::Event& event)
{
    if (event.dispatch<gfx::WindowRequestCloseEvent>([&](gfx::WindowRequestCloseEvent& event) {
        if (m_runningGame)
            m_runningGame->onWindowCloseEvent();
    })) return;
}

}