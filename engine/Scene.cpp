/*
 * ---------------------------------------------------
 * Scene.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/08 12:10:58
 * ---------------------------------------------------
 */

#include "Scene.hpp" // IWYU pragma: keep
#include "ECS/Components.hpp"
#include "ECS/ECSView.hpp"
#include "Renderer/Renderer.hpp"

namespace GE
{

Entity Scene::newEntity(const utils::String& name)
{
    Entity newEntity(m_ecsWorld, m_ecsWorld.newEntity());

    newEntity.emplace<NameComponent>(name);
    newEntity.emplace<TransformComponent>(
        math::vec3f{ 0.0F, 0.0F, 0.0F }, // position
        math::vec3f{ 0.0F, 0.0F, 0.0F }, // rotation
        math::vec3f{ 1.0F, 1.0F, 1.0F }  // scale
    );

    return newEntity;
}

void Scene::submitMeshesForRendering(Renderer& renderer)
{
    ECSView<TransformComponent, MeshComponent> view(m_ecsWorld);
    view.onEach([&](Entity entity, TransformComponent& transform, MeshComponent& mesh) {
        auto addSubMesh = [&](SubMesh& subMesh, const math::mat4x4& transform) {
            math::mat4x4 modelMatrix = transform * subMesh.transform;
            *static_cast<math::mat4x4*>(subMesh.modelMatrixBuffer->mapContent()) = modelMatrix;
            subMesh.modelMatrixBuffer->unMapContent();

            Renderer::Renderable renderable;
            renderable.vertexBuffer = subMesh.vertexBuffer;
            renderable.indexBuffer = subMesh.indexBuffer;
            renderable.modelMatrix = subMesh.modelMatrixBuffer;

            renderer.addRenderable(renderable);
        };

        if (mesh.meshID.isValid())
        {
            for (auto& subMesh : m_assetManager.getLoadedMesh(mesh.meshID).subMeshes)
                addSubMesh(subMesh, entity.worldTransform());
        }
    });
}

void Scene::submitLightsForRendering(Renderer& renderer)
{
    ECSView<TransformComponent, LightComponent> view(m_ecsWorld);
    view.onEach([&](Entity, TransformComponent& transform, LightComponent& light) {
        switch (light.type)
        {
        case LightComponent::Type::point:
            renderer.addPointLight({transform.position, light.color, light.intentsity});
            break;
        default:
            UNREACHABLE
        }
    });

}

void Scene::submitForRendering(Renderer& renderer)
{
    submitMeshesForRendering(renderer);
    submitLightsForRendering(renderer);
}

}