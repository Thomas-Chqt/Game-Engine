#include "imgui.h"
#include "imgui_render.hpp"

#include "Game-Engine/Entity.hpp"
#include "Game-Engine/Scene.hpp"

#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>

#include <optional>

namespace GE_Editor
{

namespace
{

void sceneGrapRow(GE::Entity& entity, std::optional<GE::Entity>& selectedEntity, GE::AssetManager& assetManager)
{
    ZoneScoped;
    bool node_open = false;
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;

    if (selectedEntity == entity)
        flags |= ImGuiTreeNodeFlags_Selected;

    if (entity.children().size() > 0)
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
            {
                if (auto parent = droped.parent())
                    parent->removeChild(droped);
                entity.addChild(droped);
            }
        }
        ImGui::EndDragDropTarget();
    }

    if (ImGui::IsItemClicked())
        selectedEntity = entity;

    if (ImGui::BeginPopupContextItem())
    {
        bool destroyed = false;
        menuFromPath(std::to_array<MenuItem>({
            {"Delete", [&] {
                if (selectedEntity && entity == *selectedEntity)
                    selectedEntity = std::nullopt;
                if (entity.has<GE::MeshComponent>())
                    assetManager.unloadAsset(entity.get<GE::MeshComponent>().id);
                entity.destroy();
                destroyed = true;
            }}
        }));
        ImGui::EndPopup();

        if (destroyed)
            return;
    }

    if (node_open && entity.children().size() > 0)
    {
        for (auto curr = entity.firstChild(); curr; curr = curr->nextChild())
            sceneGrapRow(*curr, selectedEntity, assetManager);
        ImGui::TreePop();
    }
}

} // namespace

void renderSceneGraph(GE::Scene& editedScene, std::optional<GE::Entity>& selectedEntity, GE::AssetManager& assetManager)
{
    ZoneScoped;
    if (ImGui::BeginChild("scene_graph_child"))
    {
        TracyCZoneN(TracyCZoneN_a, "build_root_entities", true);
        std::vector<GE::Entity> rootEntities = editedScene.ecsWorld()
            | GE::ECSView<GE::NameComponent>()
            | GE::MakeEntity{}
            | std::views::filter([](GE::Entity entity){ return entity.hasParent() == false; })
            | std::ranges::to<std::vector>();
        TracyCZoneEnd(TracyCZoneN_a);

        TracyCZoneN(TracyCZoneN_b, "render_graph_rows", true);
        for (auto entity : rootEntities)
            sceneGrapRow(entity, selectedEntity, assetManager);
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
            if(auto parent = droped.parent())
                parent->removeChild(droped);
        }
        ImGui::EndDragDropTarget();
    }
}

}
