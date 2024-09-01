/*
 * ---------------------------------------------------
 * Systems.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/15 15:01:29
 * ---------------------------------------------------
 */

#include "ECS/ECSView.hpp"
#include "Engine/EngineIntern.hpp"
#include "Game-Engine/Components.hpp"
#include "Game-Engine/Mesh.hpp"
#include "Renderer/Renderer.hpp"
#include "ECS/InternalComponents.hpp"
#include "Game-Engine/Game.hpp" // IWYU pragma: keep

namespace GE
{

void EngineIntern::scriptSystem()
{
    ECSView<ScriptComponent>(m_game->activeScene()).onEach([](Entity, ScriptComponent& scriptComponent) {
        scriptComponent.instance->onUpdate();
    });
}

Renderer::Camera EngineIntern::getActiveCameraSystem()
{
    Renderer::Camera rendererCam = { math::mat4x4(1.0), math::mat4x4(1.0) };
    ECSView<TransformComponent, CameraComponent, ActiveCameraComponent>(m_game->activeScene())
        .onFirst([&](Entity, TransformComponent& transform, CameraComponent& camera, ActiveCameraComponent&) {
            rendererCam.viewMatrix = ((math::mat4x4)transform).inversed();
            rendererCam.projectionMatrix = camera.projectionMatrix();
        });
    return rendererCam;
}

void EngineIntern::addLightsSystem()
{
    ECSView<TransformComponent, LightComponent>(m_game->activeScene())
        .onEach([&](Entity, TransformComponent& transform, LightComponent& light) {
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

void EngineIntern::addRenderableSystem()
{
    ECSView<TransformComponent, MeshComponent>(m_game->activeScene())
        .onEach([&](Entity entity, TransformComponent& transform, MeshComponent& mesh) {
            auto addSubMesh = [&](SubMesh& subMesh, const math::mat4x4& transform) {
                math::mat4x4 modelMatrix = transform * subMesh.transform;
                *static_cast<math::mat4x4*>(subMesh.modelMatrixBuffer->mapContent()) = modelMatrix;
                subMesh.modelMatrixBuffer->unMapContent();

                Renderer::Renderable renderable;
                renderable.vertexBuffer = subMesh.vertexBuffer;
                renderable.indexBuffer = subMesh.indexBuffer;
                renderable.modelMatrix = subMesh.modelMatrixBuffer;

                Renderer::shared().addRenderable(renderable);
            };

            for (auto& subMesh : ((Mesh&)mesh).subMeshes)
                addSubMesh(subMesh, entity.worldTransform());
        });
}

}