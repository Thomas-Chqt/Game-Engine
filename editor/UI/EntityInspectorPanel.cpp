/*
 * ---------------------------------------------------
 * EntityInspectorPanel.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/11/04 15:10:00
 * ---------------------------------------------------
 */

#include "UI/EntityInspectorPanel.hpp"
#include "ECS/Components.hpp"
#include "ECS/Entity.hpp"
#include "Scene.hpp"
#include "UtilsCPP/String.hpp"
#include "imgui.h"
#include <cassert>

namespace fs = std::filesystem;

namespace GE
{

template<>
void EntityInspectorPanel::componentEditWidget<NameComponent>()
{
    NameComponent& nameComponent = m_selectedEntity.get<NameComponent>();
    char buff[32];
    nameComponent.name.SAFECPY(buff);
    ImGui::InputText("Name##NameComponent_name", buff, sizeof(buff));
    nameComponent.name = utils::String(buff);
}

template<>
void EntityInspectorPanel::componentEditWidget<TransformComponent>()
{
    TransformComponent& transform = m_selectedEntity.get<TransformComponent>();
    ImGui::DragFloat3("position##TransformComponent_position", (float*)&transform.position, 0.01f, -1000.0f, 1000.0f);
    ImGui::DragFloat3("rotation##TransformComponent_rotation", (float*)&transform.rotation, 0.01f,    -2*PI,    2*PI);
    ImGui::DragFloat3("scale##TransformComponent_scale",       (float*)&transform.scale,    0.01f,     0.0f,   10.0f);
}

template<>
void EntityInspectorPanel::componentEditWidget<CameraComponent>()
{
    assert(m_scene != nullptr);

    CameraComponent& cameraComponent = m_selectedEntity.get<CameraComponent>();

    if (m_scene->activeCamera() == m_selectedEntity)
        ImGui::Text("Active");
    else if (ImGui::Button("Make active"))
        m_scene->setActiveCamera(m_selectedEntity);

    ImGui::DragFloat("fov##CameraComponent_fov",     &cameraComponent.fov,   0.01f,  -2*PI,     2*PI);
    ImGui::DragFloat("zFar##CameraComponent_zFar",   &cameraComponent.zFar,  0.01f, 0.001f, 10000.0f);
    ImGui::DragFloat("zNear##CameraComponent_zNear", &cameraComponent.zNear, 0.01f, 0.001f, 10000.0f);
}

template<>
void EntityInspectorPanel::componentEditWidget<LightComponent>()
{
    LightComponent& lightComponent = m_selectedEntity.get<LightComponent>();
    ImGui::ColorEdit3("color##LightComponent_color", (float*)&lightComponent.color);
    ImGui::DragFloat("intentsity##LightComponent_intensity", &lightComponent.intentsity, 0.01, 0.0f, 1.0f);
}

template<>
void EntityInspectorPanel::componentEditWidget<MeshComponent>()
{
    assert(m_scene != nullptr);

    MeshComponent& meshComponent = m_selectedEntity.get<MeshComponent>();

    if (ImGui::BeginCombo("Mesh##MeshComponent_mesh", meshComponent.assetId.is_nil() ? "" : m_scene->assetManager().loadedMeshes()[meshComponent.assetId].name))
    {
        for (auto& [id, mesh] : m_scene->assetManager().loadedMeshes())
        {
            const bool is_selected = (meshComponent == id);
            if (ImGui::Selectable(mesh.name, is_selected))
                meshComponent.assetId = id;
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }          
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("dnd_file"))
        {
            fs::path path = (char*)payload->Data;
            meshComponent.assetId = m_scene->assetManager().registerMesh(path);
        }
        ImGui::EndDragDropTarget();
    }
}

template<>
void EntityInspectorPanel::componentEditWidget<ScriptComponent>()
{
    ScriptComponent& scriptComponent = m_selectedEntity.get<ScriptComponent>();
    ImGui::Text("%s", scriptComponent.name.isEmpty() ? "no script" : (const char*)scriptComponent.name);
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("dnd_script"))
            scriptComponent.name = (char*)payload->Data;
        ImGui::EndDragDropTarget();
    }
}

EntityInspectorPanel::EntityInspectorPanel(Scene* scene, const Entity& selectedEntity)
    : m_scene(scene), m_selectedEntity(selectedEntity)
{
}

void EntityInspectorPanel::render()
{
    if (ImGui::Begin("Entity inspector"))
    {
        ImGui::PushItemWidth(-80);
        if (m_scene == nullptr)
            ImGui::Text("No scene edited");
        else if (m_selectedEntity == false)
            ImGui::Text("No entity selected");
        else
        {
            componentEditWidget<NameComponent>();
            if (m_selectedEntity.has<TransformComponent>() && componentEditHeader<TransformComponent>("Transform component"))
                componentEditWidget<TransformComponent>();
            if (m_selectedEntity.has<CameraComponent>() && componentEditHeader<CameraComponent>("Camera component"))
                componentEditWidget<CameraComponent>();
            if (m_selectedEntity.has<LightComponent>() && componentEditHeader<LightComponent>("Light component"))
                componentEditWidget<LightComponent>();
            if (m_selectedEntity.has<MeshComponent>() && componentEditHeader<MeshComponent>("Mesh component"))
                componentEditWidget<MeshComponent>();
            if (m_selectedEntity.has<ScriptComponent>() && componentEditHeader<ScriptComponent>("Script component"))
                componentEditWidget<ScriptComponent>();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Add component"))
                ImGui::OpenPopup("add_component_popup");
            addComponentPopUp();

            ImGui::SameLine();

            if (m_onEntityDelete && ImGui::Button("Delete enitity"))
                m_onEntityDelete();
        }
        ImGui::PopItemWidth();
    }
    ImGui::End();
}

template<typename T>
bool EntityInspectorPanel::componentEditHeader(const char* title)
{
    bool isOpen = ImGui::CollapsingHeader(title, ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_DefaultOpen);
    ImGui::SameLine(0, 30);
    if (ImGui::Button("remove##" + utils::String(title)))
        return m_selectedEntity.remove<T>(), false;
    return isOpen;
}

void EntityInspectorPanel::addComponentPopUp()
{
    if (ImGui::BeginPopup("add_component_popup"))
    {
        if (m_selectedEntity.has<TransformComponent>() == false && ImGui::Selectable("Transform component"))
            m_selectedEntity.emplace<TransformComponent>();

        if (m_selectedEntity.has<CameraComponent>() == false && ImGui::Selectable("Camera component"))
            m_selectedEntity.emplace<CameraComponent>();

        if (m_selectedEntity.has<LightComponent>() == false && ImGui::Selectable("Light component"))
            m_selectedEntity.emplace<LightComponent>();
        
        if (m_selectedEntity.has<MeshComponent>() == false && ImGui::Selectable("Mesh component"))
            m_selectedEntity.emplace<MeshComponent>();

        if (m_selectedEntity.has<ScriptComponent>() == false && ImGui::Selectable("Script component"))
            m_selectedEntity.emplace<ScriptComponent>();

        ImGui::EndPopup();
    }
}

}