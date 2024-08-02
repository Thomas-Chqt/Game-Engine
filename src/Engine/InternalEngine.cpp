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
#include "Graphics/Event.hpp"
#include "Graphics/Platform.hpp"
#include "Math/Matrix.hpp"
#include "Renderer/Renderer.hpp"
#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/Types.hpp"
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

void Engine::init()
{
    s_sharedInstance = utils::makeUnique<InternalEngine>().staticCast<Engine>();
}

InternalEngine::InternalEngine()
    : m_window(gfx::Platform::shared().newWindow(800, 600)),
      m_graphicAPI(gfx::Platform::shared().newGraphicAPI(m_window)),
      m_renderer(*m_graphicAPI)
{
    m_window->addEventCallBack(utils::Func<void(gfx::Event&)>(*this, &InternalEngine::onEvent), this);
    m_graphicAPI->initImgui();
}

void InternalEngine::runGame(utils::UniquePtr<Game>&& game)
{
    m_runningGame = std::move(game);

    m_running = true;
    while (m_running)
    {
        gfx::Platform::shared().pollEvents();

        ECSView<ScriptComponent>(*m_runningGame->m_activeECSWorld)([&](ECSWorld::EntityID, ScriptComponent& scriptComponent) {
            scriptComponent.onFrame(m_pressedKeys);
        });

        math::mat4x4 mainCameraVPMatrix = 1.0f;

        ECSView<TransformComponent, ViewPointComponent, ActiveViewPointComponent>(*m_runningGame->m_activeECSWorld)([&](ECSWorld::EntityID, TransformComponent& transformComponent, ViewPointComponent& viewPointComponent, ActiveViewPointComponent&) {
            math::mat4x4 viewMatrix = (math::mat4x4::translation(transformComponent.position) * math::mat4x4::rotation(transformComponent.rotation)).inversed();
            utils::uint32 w, h;
            m_window->getWindowSize(&w, &h);
            mainCameraVPMatrix = viewPointComponent.projectionMatrix((float)w / (float)h) * viewMatrix;
        });

        m_renderer.beginScene(mainCameraVPMatrix);

        ECSView<TransformComponent, LightSourceComponent>(*m_runningGame->m_activeECSWorld)([&](ECSWorld::EntityID, TransformComponent& transform, LightSourceComponent& lightSource) {
            if (lightSource.type == LightSourceComponent::Type::point)
                m_renderer.addPointLight(transform.position, lightSource.color, lightSource.intensity);
        });
        
        ECSView<TransformComponent, RenderableComponent>(*m_runningGame->m_activeECSWorld)([&](ECSWorld::EntityID, TransformComponent& transform, RenderableComponent& renderableComp) {
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
    if (event.dispatch<gfx::WindowRequestCloseEvent>([&](gfx::WindowRequestCloseEvent& event) {
        m_runningGame->onWindowRequestCloseEvent();
    })) return;

    if (event.dispatch<gfx::KeyDownEvent>([&](gfx::KeyDownEvent& event) {
        m_runningGame->onKeyDownEvent(event.keyCode(), event.isRepeat());
        if (event.isRepeat() == false)
            m_pressedKeys.insert(event.keyCode());
    })) return;
    
    if (event.dispatch<gfx::KeyUpEvent>([&](gfx::KeyUpEvent& event) {
        m_pressedKeys.remove(m_pressedKeys.find(event.keyCode()));
    })) return;
}

InternalEngine::~InternalEngine()
{
    m_graphicAPI->terminateImGui();
    m_window->clearCallbacks(this);
}

}