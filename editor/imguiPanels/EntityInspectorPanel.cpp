/*
 * ---------------------------------------------------
 * EntityInspectorPanel.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/13 16:35:12
 * ---------------------------------------------------
 */

#include "imguiPanels/EntityInspectorPanel.hpp"
#include "imgui.h"
#include "ECS/Components.hpp"
#include "Math/Constants.hpp"

namespace GE
{

EntityInspectorPanel::EntityInspectorPanel(Scene* scene, Entity selectedEntity)
    : m_editedScene(scene), m_selectedEntity(selectedEntity)
{
}

void EntityInspectorPanel::render()
{
    if (ImGui::Begin("Entity inspector"))
    {
        ImGui::PushItemWidth(-80);
        if (m_editedScene == nullptr)
            ImGui::Text("No scene edited");
        else if (m_selectedEntity == false)
            ImGui::Text("No entity selected");
        else
        {
            NameComponent& nameComponent = m_selectedEntity.get<NameComponent>();
            char buff[32];
            std::strncpy(buff, nameComponent.name, sizeof(buff));
            ImGui::InputText("Name", buff, sizeof(buff));
            nameComponent.name = utils::String(buff);

            if (m_selectedEntity.has<TransformComponent>())
                transformComponentEditWidget();

            if (m_selectedEntity.has<CameraComponent>())
                cameraComponentEditWidget();

            if (m_selectedEntity.has<LightComponent>())
                lightComponentEditWidget();

            if (m_selectedEntity.has<MeshComponent>())
                meshComponentEditWidget();

            if (ImGui::Button("Add component"))
                ImGui::OpenPopup("ADD_COMP_POPUP");

            addComponentPopup();
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
    if (ImGui::Button(utils::String("remove##") + utils::String(title)))
        return m_selectedEntity.remove<T>(), false;
    return isOpen;
}

void EntityInspectorPanel::transformComponentEditWidget()
{
    if (componentEditHeader<TransformComponent>("Transform component"))
    {
        TransformComponent& transform = m_selectedEntity.get<TransformComponent>();
        ImGui::DragFloat3("position", (float*)&transform.position, 0.01f, -1000.0f, 1000.0f);
        ImGui::DragFloat3("rotation", (float*)&transform.rotation, 0.01f,    -2*PI,    2*PI);
        ImGui::DragFloat3("scale",    (float*)&transform.scale,    0.01f,     0.0f,   10.0f);
    }
}

void EntityInspectorPanel::cameraComponentEditWidget()
{
    if (componentEditHeader<CameraComponent>("Camera component"))
    {
        CameraComponent& cameraComponent = m_selectedEntity.get<CameraComponent>();
        if (m_editedScene->activeCamera() == m_selectedEntity)
            ImGui::Text("Active");
        else if (ImGui::Button("Make active"))
            m_editedScene->setActiveCamera(m_selectedEntity);
        ImGui::DragFloat("fov",   &cameraComponent.fov,   0.01f,  -2*PI,     2*PI);
        ImGui::DragFloat("zFar",  &cameraComponent.zFar,  0.01f, 0.001f, 10000.0f);
        ImGui::DragFloat("zNear", &cameraComponent.zNear, 0.01f, 0.001f, 10000.0f);
    }
}

void EntityInspectorPanel::lightComponentEditWidget()
{
    if (componentEditHeader<LightComponent>("Light component"))
    {
        LightComponent& lightComponent = m_selectedEntity.get<LightComponent>();
        ImGui::ColorEdit3("color", (float*)&lightComponent.color);
        ImGui::DragFloat("intentsity", &lightComponent.intentsity, 0.01, 0.0f, 1.0f);
    }
}

void EntityInspectorPanel::meshComponentEditWidget()
{
    if (componentEditHeader<MeshComponent>("Mesh component"))
    {
        MeshComponent& meshComponent = m_selectedEntity.get<MeshComponent>();
        ImGui::Text("mesh: %s",(char*)m_editedScene->assetManager().getMeshName(meshComponent.meshID));
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MESH_DND"))
            {
                IM_ASSERT(payload->DataSize == sizeof(xg::Guid));
                meshComponent.meshID = *(xg::Guid*)payload->Data;
            }
            ImGui::EndDragDropTarget();
        }
    }
}

void EntityInspectorPanel::addComponentPopup()
{
    if (ImGui::BeginPopup("ADD_COMP_POPUP"))
    {
        if (m_selectedEntity.has<TransformComponent>() == false && ImGui::Selectable("Transform component"))
            m_selectedEntity.emplace<TransformComponent>(math::vec3f{0,0,0},math::vec3f{0,0,0},math::vec3f{1,1,1});

        if (m_selectedEntity.has<CameraComponent>() == false && ImGui::Selectable("Camera component"))
            m_selectedEntity.emplace<CameraComponent>((float)(60 * (PI / 180.0F)), 10000.0f, 0.01f);

        if (m_selectedEntity.has<LightComponent>() == false && ImGui::Selectable("Light component"))
            m_selectedEntity.emplace<LightComponent>(LightComponent::Type::point, WHITE3, 1.0f);

        if (m_selectedEntity.has<MeshComponent>() == false && ImGui::Selectable("Mesh component"))
            m_selectedEntity.emplace<MeshComponent>();

        ImGui::EndPopup();
    }
}

}