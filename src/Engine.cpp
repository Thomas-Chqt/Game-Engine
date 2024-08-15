/*
 * ---------------------------------------------------
 * Engine.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/06/20 17:15:45
 * ---------------------------------------------------
 */

#include "AssetManager/MeshImpl.hpp"
#include "ECS/ECSView.hpp"
#include "Game-Engine/AssetManager.hpp"
#include "Game-Engine/Components.hpp"
#include "Game-Engine/Engine.hpp"
#include "Game-Engine/Entity.hpp"
#include "Game-Engine/Game.hpp"
#include "Graphics/Event.hpp"
#include "Graphics/Platform.hpp"
#include "Math/Matrix.hpp"
#include "Renderer/GPURessourceManager.hpp"
#include "Renderer/Renderer.hpp"
#include "Scene/InternalComponents.hpp"
#include "UtilsCPP/Func.hpp"
#include <utility>

namespace GE
{

static void scriptSystem(ECSView<ScriptComponent> view)
{
    view.onEach([](Entity, ScriptComponent& scriptComponent) {
        scriptComponent.instance->onUpdate();
    });
}

static Renderer::Camera getActiveCameraSystem(ECSView<TransformComponent, CameraComponent, ActiveCameraComponent> view)
{
    Renderer::Camera rendererCam = { math::mat4x4(1.0), math::mat4x4(1.0) };
    view.onFirst([&](Entity, TransformComponent& transform, CameraComponent& camera, ActiveCameraComponent&){
        rendererCam.viewMatrix = ((math::mat4x4)transform).inversed();
        rendererCam.projectionMatrix = camera.projectionMatrix;
    });
    return rendererCam;;
}

static void addLightsSystem(ECSView<TransformComponent, LightComponent> view)
{
    view.onEach([](Entity, TransformComponent& transform, LightComponent& light){
        switch (light.type)
        {
        case LightComponent::Type::point:
            Renderer::shared().addPointLight({transform.position, light.color, light.intentsity});
            break;
        default:
            UNREACHABLE
        }
    });
}

static void addRenderableSystel(ECSView<TransformComponent, MeshComponent> view)
{
    view.onEach([](Entity, TransformComponent& transform, MeshComponent& meshComponent) {
        utils::SharedPtr<MeshImpl> mesh = meshComponent.mesh.forceDynamicCast<MeshImpl>();

        auto addSubMesh = [&](SubMeshImpl& subMesh, const math::mat4x4& transform) {
            math::mat4x4 modelMatrix = transform * subMesh.transform;
            *static_cast<math::mat4x4*>(subMesh.modelMatrixBuffer->mapContent()) = modelMatrix;
            subMesh.modelMatrixBuffer->unMapContent();

            Renderer::Renderable renderable;
            renderable.vertexBuffer = subMesh.vertexBuffer;
            renderable.indexBuffer = subMesh.indexBuffer;
            renderable.modelMatrix = subMesh.modelMatrixBuffer;

            Renderer::shared().addRenderable(renderable);
        };

        for (auto& subMesh : mesh->subMeshes)
            addSubMesh(subMesh, transform);
    });
}

void Engine::runGame(utils::UniquePtr<Game>&& game)
{
    m_runningGame = std::move(game);

    m_runningGame->onSetup();

    m_running = true;
    while (m_running)
    {
        gfx::Platform::shared().pollEvents();

        m_runningGame->onUpdate();

        scriptSystem(m_runningGame->activeScene());

        Renderer::Camera rendererCam = getActiveCameraSystem(m_runningGame->activeScene());

        Renderer::shared().beginScene(rendererCam);
        {
            addLightsSystem(m_runningGame->activeScene());
            addRenderableSystel(m_runningGame->activeScene());
        }
        Renderer::shared().endScene();

        Renderer::shared().render();
    }
}

Engine::~Engine()
{
    AssetManager::terminate();
    GPURessourceManager::terminate();
    Renderer::terminate();

    gfx::Platform::shared().clearCallbacks(this);
    gfx::Platform::terminate();
}

Engine::Engine()
{
    gfx::Platform::init();
    gfx::Platform::shared().addEventCallBack(utils::Func<void(gfx::Event&)>(*this, &Engine::onEvent), this);

    Renderer::init();
    GPURessourceManager::init();
    AssetManager::init();

    m_mainWindow = gfx::Platform::shared().newWindow(800, 600);
    Renderer::shared().setWindow(m_mainWindow);
}

void Engine::onEvent(gfx::Event& event)
{
    event.dispatch<gfx::KeyDownEvent>([&](gfx::KeyDownEvent& event) {
        if (event.isRepeat() == false)
            m_pressedKeys.insert(event.keyCode());
    });
    
    event.dispatch<gfx::KeyUpEvent>([&](gfx::KeyUpEvent& event) {
        m_pressedKeys.remove(m_pressedKeys.find(event.keyCode()));
    });

    if (m_runningGame)
        m_runningGame->onEvent(event);
}

}