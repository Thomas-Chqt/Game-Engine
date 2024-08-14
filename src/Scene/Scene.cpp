/*
 * ---------------------------------------------------
 * Scene.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/07 22:01:50
 * ---------------------------------------------------
 */

#include "Game-Engine/Scene.hpp"
#include "AssetManager/MeshImpl.hpp"
#include "Game-Engine/Mesh.hpp"
#include "ECS/ECSView.hpp"
#include "Game-Engine/Components.hpp"
#include "Game-Engine/Entity.hpp"
#include "Math/Matrix.hpp"
#include "Renderer/GPURessourceManager.hpp"
#include "Renderer/Renderer.hpp"
#include "Scene/InternalComponents.hpp"
#include "UtilsCPP/Dictionary.hpp"
#include "UtilsCPP/String.hpp"

namespace GE
{

Entity Scene::newEntity(const utils::String& name)
{
    Entity newEntity = Entity(m_world, m_world.newEntity());
    newEntity.emplace<NameComponent>(name);
    newEntity.emplace<TransformComponent>(
        math::vec3f{ 0.0F, 0.0F, 0.0F }, // position
        math::vec3f{ 0.0F, 0.0F, 0.0F }, // rotation
        math::vec3f{ 1.0F, 1.0F, 1.0F }  // scale
    );
    return newEntity;
}

void Scene::onUpdate()
{
    ECSView<ScriptComponent>(m_world).onEach([](Entity, ScriptComponent& scriptComponent) {
        scriptComponent.instance->onUpdate();
    });

    Renderer::Camera rendererCam = { math::mat4x4(1.0), math::mat4x4(1.0) };
    ECSView<TransformComponent, CameraComponent>(m_world).onFirst([&](Entity, TransformComponent& transform, CameraComponent& camera) {
        rendererCam.viewMatrix = ((math::mat4x4)transform).inversed();
        rendererCam.projectionMatrix = camera.projectionMatrix;
    });

    Renderer::shared().beginScene(rendererCam);

    ECSView<TransformComponent, LightComponent>(m_world).onEach([](Entity, TransformComponent& transform, LightComponent& light){
        switch (light.type)
        {
        case LightComponent::Type::point:
            Renderer::shared().addPointLight({transform.position, light.color, light.intentsity});
            break;
        default:
            UNREACHABLE
        }
    });

    ECSView<TransformComponent, MeshComponent>(m_world).onEach([](Entity, TransformComponent& transform, MeshComponent& mesh) {
        utils::Array<GPURessource<gfx::Buffer>>& buffers = mesh.mesh.forceDynamicCast<MeshImpl>()->buffers;

        utils::Func<void(SubMesh&, const math::mat4x4&)> addSubMesh = [&](SubMesh& subMesh, const math::mat4x4& transform) {
            math::mat4x4 modelMatrix = transform * subMesh.transform;
            *static_cast<math::mat4x4*>(buffers[subMesh.modelMatrixBufferIdx]->mapContent()) = modelMatrix;
            buffers[subMesh.modelMatrixBufferIdx]->unMapContent();

            Renderer::Renderable renderable;
            renderable.vertexBuffer = buffers[subMesh.vertexBufferIdx];
            renderable.indexBuffer = buffers[subMesh.indexBufferIdx];
            renderable.modelMatrix = buffers[subMesh.modelMatrixBufferIdx];

            Renderer::shared().addRenderable(renderable);
        };

        for (auto& subMesh : ((utils::SharedPtr<Mesh>)mesh)->subMeshes)
            addSubMesh(subMesh, transform);
    });

    Renderer::shared().endScene();
    Renderer::shared().render();
}

}