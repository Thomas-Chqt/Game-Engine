#include "Game-Engine/Components.hpp"
#include "imgui.h"
#include "imgui_render.hpp"

#include "Game-Engine/Entity.hpp"
#include "Game-Engine/Scene.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>

#include <optional>
#include <ranges>
#include <vector>

namespace GE_Editor
{

namespace
{

void reparentKeepingWorldTransform(GE::Entity& entity, std::optional<GE::Entity> newParent)
{
    assert(entity.has<GE::TransformComponent>());

    const glm::mat4 worldTransform = entity.worldTransform();

    if (auto currentParent = entity.parent())
        currentParent->removeChild(entity);
    if (newParent)
        newParent->addChild(entity);

    const glm::mat4 parentWorldTransform = newParent && newParent->has<GE::TransformComponent>()
        ? newParent->worldTransform()
        : glm::mat4(1.0f);
    const glm::mat4 localTransform = glm::inverse(parentWorldTransform) * worldTransform;

    GE::TransformComponent& transform = entity.get<GE::TransformComponent>();
    [[maybe_unused]] glm::vec3 skew;
    [[maybe_unused]] glm::vec4 perspective;
    [[maybe_unused]] const bool decomposed = glm::decompose(
        localTransform,
        transform.scale,
        transform.rotation,
        transform.position,
        skew,
        perspective
    );
    assert(decomposed);

    entity.updateTransformHierarchy();
}

bool sceneGrapRow(GE::Entity& entity, std::optional<GE::Entity>& selectedEntity, GE::AssetManager& assetManager)
{
    ZoneScoped;
    bool node_open = false;
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;

    if (selectedEntity == entity)
        flags |= ImGuiTreeNodeFlags_Selected;

    if (entity.hasChild())
        node_open = ImGui::TreeNodeEx(reinterpret_cast<void*>(entity.entityId), flags, "%s", entity.name().c_str()); // NOLINT
    else
    {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        ImGui::TreeNodeEx(reinterpret_cast<void*>(entity.entityId), flags, "%s", entity.name().c_str()); // NOLINT
    }

    if (ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("dnd_entity", &entity, sizeof(GE::Entity));
        ImGui::TextUnformatted(entity.name().c_str());
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("dnd_entity"); payload && payload->IsDelivery())
        {
            assert(payload->DataSize == sizeof(GE::Entity));
            GE::Entity& droped = *reinterpret_cast<GE::Entity*>(payload->Data); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
            if (droped.isParentOf(entity) == false)
                reparentKeepingWorldTransform(droped, entity);
        }
        ImGui::EndDragDropTarget();
    }

    if (ImGui::IsItemClicked())
        selectedEntity = entity;

    if (ImGui::BeginPopupContextItem())
    {
        bool destroyed = false;
        std::vector<MenuItem> items = {
            {"Delete", [&] {
                if (selectedEntity && entity == *selectedEntity)
                    selectedEntity = std::nullopt;
                if (entity.has<GE::MeshComponent>())
                    assetManager.unloadAsset(entity.get<GE::MeshComponent>().id);
                entity.destroy();
                destroyed = true;
            }},
        };
        if (entity.hasChild()) {
            items.emplace_back("Delete all", [&] {
                auto cascadeDestroy = [&selectedEntity, &assetManager](this auto&& self, GE::Entity entity) -> void {
                    for (auto& child : entity.children())
                        self(child);
                    if (selectedEntity && entity == *selectedEntity)
                        selectedEntity = std::nullopt;
                    if (entity.has<GE::MeshComponent>())
                        assetManager.unloadAsset(entity.get<GE::MeshComponent>().id);
                    entity.destroy();
                };
                cascadeDestroy(entity);
                destroyed = true;
            });
        }

        menuFromPath(items);

        ImGui::EndPopup();

        if (destroyed)
            return node_open;
    }

    if (node_open && entity.hasChild())
    {
        for (auto& child : entity.children()) {
            if (sceneGrapRow(child, selectedEntity, assetManager))
                ImGui::TreePop();
        }
    }

    return node_open;
}

} // namespace

void renderSceneGraph(GE::Scene& editedScene, std::optional<GE::Entity>& selectedEntity, GE::AssetManager& assetManager)
{
    ZoneScoped;
    if (ImGui::BeginChild("scene_graph_child"))
    {
        TracyCZoneN(TracyCZoneN_a, "build_root_entities", true);
        std::vector<GE::Entity> rootEntities = editedScene.ecsWorld()
            | GE::ECSView<GE::NameComponent>().without<GE::ParentComponent>()
            | GE::MakeEntity{}
            | std::ranges::to<std::vector>();
        TracyCZoneEnd(TracyCZoneN_a);

        TracyCZoneN(TracyCZoneN_b, "render_graph_rows", true);
        for (auto entity : rootEntities) {
            if (sceneGrapRow(entity, selectedEntity, assetManager))
                ImGui::TreePop();
        }
        TracyCZoneEnd(TracyCZoneN_b);

        if (ImGui::BeginPopupContextWindow("scene_graph_context", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            menuFromPath(std::to_array<MenuItem>({
                {"Add/Cube", [&] {
                    GE::Entity newEntity = editedScene.newEntity("cube");
                    newEntity.emplace<GE::TransformComponent>();
                    newEntity.emplace<GE::MeshComponent>(GE::BUILT_IN_CUBE_ID);
                    assetManager.loadAsset(GE::BUILT_IN_CUBE_ID);
                    selectedEntity = newEntity;
                }},
                {"Add/Light", [&] {
                    GE::Entity newEntity = editedScene.newEntity("light");
                    newEntity.emplace<GE::TransformComponent>();
                    newEntity.emplace<GE::LightComponent>();
                    selectedEntity = newEntity;
                }},
                {"Add/Camera", [&] {
                    GE::Entity newEntity = editedScene.newEntity("camera");
                    newEntity.emplace<GE::TransformComponent>();
                    newEntity.emplace<GE::CameraComponent>();
                    selectedEntity = newEntity;
                }}
            }));
            ImGui::EndPopup();
        }
        ImGui::EndChild();
    }
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("dnd_entity"); payload && payload->IsDelivery()) {
            assert(payload->DataSize == sizeof(GE::Entity));
            GE::Entity& droped = *reinterpret_cast<GE::Entity*>(payload->Data); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
            if (droped.parent())
                reparentKeepingWorldTransform(droped, std::nullopt);
        }
        ImGui::EndDragDropTarget();
    }
}

}
