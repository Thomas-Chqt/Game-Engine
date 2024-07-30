/*
 * ---------------------------------------------------
 * InternalEngine.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/30 16:52:29
 * ---------------------------------------------------
 */

#include "Engine/InternalEngine.hpp"
#include "Game-Engine/Components.hpp"
#include "Game-Engine/ECSWorld.hpp"
#include "Game-Engine/Engine.hpp"
#include "Game-Engine/Game.hpp"
#include "Game-Engine/Mesh.hpp"
#include "Graphics/Platform.hpp"
#include "Math/Matrix.hpp"
#include "Renderer/Renderer.hpp"
#include "UtilsCPP/Types.hpp"
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

void Engine::init()
{
    s_sharedInstance = utils::makeUnique<InternalEngine>().staticCast<Engine>();
}

InternalEngine::InternalEngine()
{
    gfx::Platform::init();

    m_window = gfx::Platform::shared().newWindow(800, 600);
    m_graphicAPI = gfx::Platform::shared().newGraphicAPI(m_window);
    m_window->addEventCallBack(utils::Func<void(gfx::Event&)>(*this, &InternalEngine::onEvent), this);
    m_renderer.setGraphicAPI(m_graphicAPI);
}

void InternalEngine::runGame(utils::UniquePtr<Game>&& game)
{
    m_game = std::move(game);

    m_running = true;
    while (m_running)
    {
        gfx::Platform::shared().pollEvents();

        math::mat4x4 mainCameraVPMatrix = 1.0f;

        ECSWorld::View<TransformComponent, ViewPointComponent, ActiveViewPointComponent>(*m_game->m_activeECSWorld)
            .onFirst([&](TransformComponent& transform, ViewPointComponent& viewPoint, ActiveViewPointComponent&) {
                math::mat4x4 viewMatrix = (math::mat4x4::translation(transform.position) * math::mat4x4::rotation(transform.rotation)).inversed();
                utils::uint32 w, h;
                m_window->getWindowSize(&w, &h);
                mainCameraVPMatrix = viewPoint.projectionMatrix((float)w / (float)h) * viewMatrix;
            });

        m_renderer.beginScene(mainCameraVPMatrix);

        ECSWorld::View<TransformComponent, LightSourceComponent>(*m_game->m_activeECSWorld).foreach([&](TransformComponent& transform, LightSourceComponent& lightSource) {
            if (lightSource.type == LightSourceComponent::Type::point)
                m_renderer.addPointLight(transform.position, lightSource.color, lightSource.intensity);
        });

        ECSWorld::View<TransformComponent, RenderableComponent>(*m_game->m_activeECSWorld).foreach([&](TransformComponent& transform, RenderableComponent& renderableComp) {
            utils::Func<void(SubMesh&, const math::mat4x4&)> addSubMesh = [&](SubMesh& subMesh, const math::mat4x4& transform) {
                math::mat4x4 modelMatrix = transform * subMesh.transform;
                if (subMesh.vertexBuffer && subMesh.indexBuffer)
                {
                    subMesh.modelMatrix.map();
                    subMesh.modelMatrix.content() = modelMatrix;
                    subMesh.modelMatrix.unmap();

                    Renderer::Renderable renderable;
                    renderable.vertexBuffer = subMesh.vertexBuffer;
                    renderable.indexBuffer = subMesh.indexBuffer;
                    renderable.modelMatrix = subMesh.modelMatrix.buffer();

                    m_renderer.addRenderable(renderable);
                }
            };
            for (auto& subMesh : renderableComp.mesh.subMeshes)
                addSubMesh(subMesh, transform.modelMatrix());
        });

        m_renderer.endScene();
    }
}

void InternalEngine::onEvent(gfx::Event& event)
{
    event.dispatch<gfx::WindowRequestCloseEvent>([&](gfx::WindowRequestCloseEvent& event) {
        m_game->onWindowCloseEvent();
    });
}

InternalEngine::~InternalEngine()
{
    m_window->clearCallbacks(this);
    gfx::Platform::terminate();
}

}