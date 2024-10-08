/*
 * ---------------------------------------------------
 * ImGuiRender.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/15 15:18:37
 * ---------------------------------------------------
 */

#include "ECS/ECSView.hpp"
#include "Engine/EngineIntern.hpp"
#include "Game-Engine/Components.hpp"
#include "Game-Engine/Game.hpp" // IWYU pragma: keep
#include "Math/Constants.hpp"
#include "ECS/InternalComponents.hpp"
#include <imgui.h>
#include <cstring>

namespace GE
{

void EngineIntern::onImGuiRender()
{
    if (ImGui::Begin("FPS"))
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::End();

    drawSceneGraphWindow();
    drawEntityInspectorWindow();

    assert(m_game);
    m_game->onImGuiRender();
}

void EngineIntern::drawSceneGraphWindow()
{
    utils::Func<void(Entity)> sceneGraphEntityRow = [&](Entity entity) {
        bool node_open = false;
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
                                   ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                   ImGuiTreeNodeFlags_SpanAvailWidth |
                                   ImGuiTreeNodeFlags_DefaultOpen;

        if (m_selectedEntity == entity)
            flags |= ImGuiTreeNodeFlags_Selected;

        if (entity.has<HierarchicalComponent>() == true && entity.firstChild())
            node_open = ImGui::TreeNodeEx(entity.imGuiID(), flags, "%s", (const char*)entity.name());
        else
        {
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            ImGui::TreeNodeEx(entity.imGuiID(), flags, "%s", (const char*)entity.name());
        }

        if (ImGui::IsItemClicked())
            m_selectedEntity = entity;

        if (node_open)
        {
            for (Entity curr = entity.firstChild(); curr; curr = curr.nextChild() )
                sceneGraphEntityRow(curr);
            ImGui::TreePop();
        }
    };

    if (ImGui::Begin("Scene graph"))
    {
        ECSView<NameComponent>(m_game->activeScene()).onEach([&](Entity entity, NameComponent &) {
            if (entity.has<HierarchicalComponent>() == false || entity.parent() == false)
                sceneGraphEntityRow(entity);
        });
    }
    ImGui::End();
}

void EngineIntern::drawEntityInspectorWindow()
{
    if (ImGui::Begin("Entity inspector"))
    {
        if (m_selectedEntity == false)
            ImGui::Text("No entity selected");
        else
        {
            ImGui::PushItemWidth(-100);

            NameComponent& nameComponent = m_selectedEntity.get<NameComponent>();
            char buff[32];
            std::strncpy(buff, nameComponent.name, sizeof(buff));
            ImGui::InputText("Name", buff, sizeof(buff));
            nameComponent.name = utils::String(buff);
            
            if (m_selectedEntity.has<TransformComponent>())
            {
                ImGui::SeparatorText("Transform Component");

                TransformComponent& transform = m_selectedEntity.get<TransformComponent>();
                ImGui::DragFloat3("position", (float*)&transform.position, 0.01f, -1000.0f, 1000.0f);
                ImGui::DragFloat3("rotation", (float*)&transform.rotation, 0.01f,    -2*PI,    2*PI);
                ImGui::DragFloat3("scale",    (float*)&transform.scale,    0.01f,     0.0f,   10.0f);
            }

            if (m_selectedEntity.has<CameraComponent>())
            {
                ImGui::SeparatorText("Camera Component");

                CameraComponent& cameraComponent = m_selectedEntity.get<CameraComponent>();
                if (m_selectedEntity.has<ActiveCameraComponent>())
                    ImGui::Text("Active");
                else if (ImGui::Button("Make active"))
                {
                    ECSView<ActiveCameraComponent>(m_game->activeScene()).onFirst([](Entity entity, ActiveCameraComponent&){ 
                        entity.remove<ActiveCameraComponent>();
                    });
                    m_selectedEntity.emplace<ActiveCameraComponent>();
                }
                ImGui::DragFloat("fov",   &cameraComponent.fov,   0.01f,  -2*PI,     2*PI);
                ImGui::DragFloat("zFar",  &cameraComponent.zFar,  0.01f, 0.001f, 10000.0f);
                ImGui::DragFloat("zNear", &cameraComponent.zNear, 0.01f, 0.001f, 10000.0f);
            }

            if (m_selectedEntity.has<LightComponent>())
            {
                ImGui::SeparatorText("Light Component");

                LightComponent& lightComponent = m_selectedEntity.get<LightComponent>();
                ImGui::ColorEdit3("color", (float*)&lightComponent.color);
                ImGui::DragFloat("intentsity", &lightComponent.intentsity, 0.01, 0.0f, 1.0f);
            }
            
            if (m_selectedEntity.has<MeshComponent>())
            {
                ImGui::SeparatorText("Mesh Component");

                MeshComponent meshComponent = m_selectedEntity.get<MeshComponent>();
                ImGui::Text("%s", (char*)meshComponent.mesh.name);
                if (ImGui::TreeNode("Sub Meshes"))
                {
                    for (auto& subMesh : meshComponent.mesh.subMeshes)
                        ImGui::Text("%s", (char*)subMesh.name);
                    ImGui::TreePop();
                }
            }

            ImGui::PopItemWidth();
        }
    }
    ImGui::End();
}

}