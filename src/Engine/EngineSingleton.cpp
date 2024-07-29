/*
 * ---------------------------------------------------
 * EngineSingleton.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/06/20 17:15:45
 * ---------------------------------------------------
 */

#include "Engine/EngineSingleton.hpp"
#include "Game-Engine/Components.hpp"
#include "Game-Engine/ECSWorld.hpp"
#include "Graphics/Event.hpp"
#include "Graphics/Platform.hpp"
#include "Game-Engine/Engine.hpp"
#include "Math/Matrix.hpp"
#include "Renderer/Renderer.hpp"
#include "UtilsCPP/Func.hpp"
#include "Game-Engine/ECSWorldView.hpp"

namespace GE
{

utils::UniquePtr<EngineSingleton> EngineSingleton::s_sharedInstance;

void EngineSingleton::onEvent(gfx::Event& event)
{
    event.dispatch<gfx::WindowResizeEvent>([&](gfx::WindowResizeEvent& event){
        m_aspectRatio = (float)event.width() / (float)event.height();
    });

    event.dispatch<gfx::WindowRequestCloseEvent>([this](gfx::WindowRequestCloseEvent& event) {
        m_game->onWindowCloseEvent();
    });
}

void EngineSingleton::runGame(utils::UniquePtr<Game>&& game)
{
    m_game = std::move(game);

    m_gameWindow = gfx::Platform::shared().newWindow(m_game->m_windowWidth, m_game->m_windowHeight);
    m_gameWindow->addEventCallBack(utils::Func<void(gfx::Event&)>(*this, &EngineSingleton::onEvent));

    m_aspectRatio = (float)m_game->m_windowWidth / (float)m_game->m_windowHeight;

    m_gameRenderer = Renderer(gfx::Platform::shared().newGraphicAPI(m_gameWindow));

    m_running = true;
    while (m_running)
    {
        gfx::Platform::shared().pollEvents();

        EntityID mainCamera = ECSWorldView<TransformComponent, ViewPointComponent, ActiveViewPointComponent>(*m_game->m_activeECSWorld).first();
        TransformComponent& mainCameraTransform = m_game->m_activeECSWorld->getComponent<TransformComponent>(mainCamera);
        ViewPointComponent& mainCameraViewPoint = m_game->m_activeECSWorld->getComponent<ViewPointComponent>(mainCamera);
        math::mat4x4 mainCameraViewMatrix = (math::mat4x4::translation(mainCameraTransform.position) * math::mat4x4::rotation(mainCameraTransform.rotation)).inversed();
        math::mat4x4 mainCameraVPMatrix = mainCameraViewPoint.projectionMatrix(m_aspectRatio) * mainCameraViewMatrix;

        m_gameRenderer.beginScene(mainCameraTransform.position, mainCameraVPMatrix);
        m_gameRenderer.endScene();
    }
}

void Engine::terminateGame()
{
    EngineSingleton::shared().terminateGame();
}

}