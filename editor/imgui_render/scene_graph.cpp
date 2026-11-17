#include "Game-Engine/Components.hpp"
#include "imgui.h"
#include "imgui_render.hpp"
#include "EditorTheme.hpp"
#include "ui_state.hpp"

#include "Game-Engine/Entity.hpp"
#include "Game-Engine/Scene.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>

#include <algorithm>
#include <cctype>
#include <optional>
#include <string>
#include <ranges>
#include <vector>

namespace GE_Editor
{

void SceneGraphState::commit()
{
    // Keep the previous name when an empty or whitespace-only edit is submitted.
    if (entity && entity->registry->isValidEntityID(entity->entityId) && std::ranges::any_of(std::string_view(buffer.data()), [](unsigned char c) { return !std::isspace(c); }))
        entity->get<GE::NameComponent>().name = buffer.data();
    entity.reset();
}

void SceneGraphState::begin(GE::Entity target)
{
    if (entity) commit();
    entity = target;
    buffer.assign(std::max<std::size_t>(256, target.name().size()+64), '\0');
    std::copy(target.name().begin(), target.name().end(), buffer.begin());
    focus = true;
}

namespace
{

void renameField(GE::Entity entity, SceneGraphState& rename, ImVec2 min, ImVec2 max)
{
    rename.rendered = true;
    const float labelX = min.x + ImGui::GetTreeNodeToLabelSpacing();
    // Share the tree row's line height and let InputText advance the cursor normally.
    ImGui::SameLine();
    ImGui::SetCursorScreenPos(ImVec2(labelX, min.y));
    ImGui::PushID(uuids::to_string(entity.entityId).c_str());
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(Theme::hierarchyRenamePaddingX, 0));
    ImGui::SetNextItemWidth(std::max(1.0f, max.x-labelX));
    const bool firstFrame = rename.focus;
    if (firstFrame) { ImGui::SetKeyboardFocusHere(); rename.focus = false; }
    const bool confirmed = ImGui::InputText("##rename", rename.buffer.data(), rename.buffer.size(),
        ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackResize,
        [](ImGuiInputTextCallbackData* data) {
            if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
                auto& buffer = *static_cast<std::vector<char>*>(data->UserData);
                buffer.resize(data->BufTextLen+1);
                data->Buf = buffer.data();
            }
            return 0;
        }, &rename.buffer);
    if (confirmed || (!firstFrame && ImGui::IsItemDeactivated())) rename.commit();
    ImGui::PopStyleVar();
    ImGui::PopID();
}

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

bool sceneGrapRow(GE::Entity& entity, std::optional<GE::Entity>& selectedEntity, GE::AssetManager& assetManager, SceneGraphState& rename)
{
    ZoneScoped;
    bool node_open = false;
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;

    if (selectedEntity == entity)
        flags |= ImGuiTreeNodeFlags_Selected;

    const bool editing = rename.entity == entity;
    if (editing) flags |= ImGuiTreeNodeFlags_AllowOverlap;
    if (entity.hasChild())
        node_open = ImGui::TreeNodeEx(uuids::to_string(entity.entityId).c_str(), flags, "%s", editing ? "" : entity.name().c_str());
    else {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        ImGui::TreeNodeEx(uuids::to_string(entity.entityId).c_str(), flags, "%s", editing ? "" : entity.name().c_str());
    }
    const auto rowMin = ImGui::GetItemRectMin();
    const auto rowMax = ImGui::GetItemRectMax();
    const bool clicked = ImGui::IsItemClicked();
    const bool doubleClickedName = ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)
        && ImGui::GetMousePos().x >= rowMin.x + ImGui::GetTreeNodeToLabelSpacing();
    if (!editing && doubleClickedName) {
        rename.begin(entity);
        selectedEntity = entity;
    }

    if (rename.entity != entity && ImGui::BeginDragDropSource())
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

    if (clicked) selectedEntity = entity;

    if (rename.entity != entity && ImGui::BeginPopupContextItem())
    {
        bool destroyed = false;
        std::vector<MenuItem> items = {
            {"Rename", [&] { rename.begin(entity); selectedEntity = entity; }},
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

    if (rename.entity == entity) renameField(entity, rename, rowMin, rowMax);

    if (node_open && entity.hasChild())
    {
        for (auto& child : entity.children()) {
            if (sceneGrapRow(child, selectedEntity, assetManager, rename))
                ImGui::TreePop();
        }
    }

    return node_open;
}

} // namespace

void renderSceneGraph(GE::Scene& editedScene, std::optional<GE::Entity>& selectedEntity, GE::AssetManager& assetManager, SceneGraphState& rename)
{
    ZoneScoped;
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, Theme::hierarchyIndentSpacing);
    if (rename.entity && (rename.entity->registry != &editedScene.ecsRegistry()
        || !editedScene.ecsRegistry().isValidEntityID(rename.entity->entityId)))
        rename.entity.reset();
    rename.rendered = false;
    if (ImGui::BeginChild("scene_graph_child"))
    {
        TracyCZoneN(TracyCZoneN_a, "build_root_entities", true);
        std::vector<GE::Entity> rootEntities = editedScene.ecsRegistry()
            | GE::ECSView<GE::NameComponent, GE::ExcludeComponents<GE::ParentComponent>>()
            | GE::MakeEntity{}
            | std::ranges::to<std::vector>();
        TracyCZoneEnd(TracyCZoneN_a);

        TracyCZoneN(TracyCZoneN_b, "render_graph_rows", true);
        for (auto entity : rootEntities) {
            if (sceneGrapRow(entity, selectedEntity, assetManager, rename))
                ImGui::TreePop();
        }
        TracyCZoneEnd(TracyCZoneN_b);
        if (rename.entity && !rename.rendered) rename.commit();

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
    ImGui::PopStyleVar();
}

}
