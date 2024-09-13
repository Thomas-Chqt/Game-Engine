/*
 * ---------------------------------------------------
 * ImGuiPanels.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/15 15:18:37
 * ---------------------------------------------------
 */

#if 0
#include "ECS/Components.hpp"
#include "Graphics/Texture.hpp"
#include "Math/Constants.hpp"
#include "Math/Vector.hpp"
#include "Scene.hpp"
#include "UtilsCPP/Array.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include <imgui.h>
#include <cstring>
#include "Editor.hpp"
#include "UtilsCPP/String.hpp"

namespace GE
{

void Editor::drawViewportPanel()
{
    if (ImGui::Begin("viewport"))
    {
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        viewportSize.x = viewportSize.x == 0 ? 1 : viewportSize.x;
        viewportSize.y = viewportSize.y == 0 ? 1 : viewportSize.y;
        if (math::vec2f{viewportSize.x, viewportSize.y} != m_viewportPanelSize)
            m_viewportPanelSizeIsDirty = true;
        m_viewportPanelSize = math::vec2f{viewportSize.x, viewportSize.y};

        utils::SharedPtr<gfx::Texture> colorTexture = m_viewportFBuff->colorTexture();
        ImGui::Image(colorTexture->imguiTextureId(), viewportSize, colorTexture->imguiUV0(), colorTexture->imguiUV1());
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_DND"))
            {
                IM_ASSERT(payload->DataSize == sizeof(Scene*));
                editScene(*(Scene**)payload->Data);
            }
            ImGui::EndDragDropTarget();
        }
    }
    ImGui::End();
}

void Editor::drawSceneGraphPanel()
{
    utils::Func<void(Entity)> sceneGraphEntityRow = [&](Entity entity) {
        bool node_open = false;
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow       |
                                   ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                   ImGuiTreeNodeFlags_SpanAvailWidth    |
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
        if (m_editedScene)
        {
            m_editedScene->forEachNamedEntity([&](Entity entity, NameComponent&) {
                if (entity.has<HierarchicalComponent>() == false || entity.parent() == false)
                    sceneGraphEntityRow(entity);
            });
        }
        else
        {
            ImGui::Text("No scene edited");
        }
    }
    ImGui::End();
}

void Editor::drawEntityInspectorPanel()
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
                if (m_editedScene->activeCamera() == m_selectedEntity)
                    ImGui::Text("Active");
                else if (ImGui::Button("Make active"))
                    m_editedScene->setActiveCamera(m_selectedEntity);
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

            utils::Array<utils::String> missingComponents;
            if (m_selectedEntity.has<TransformComponent>() == false)
                missingComponents.append("Transform component");
            if (m_selectedEntity.has<CameraComponent>() == false)
                missingComponents.append("Camera component");
            if (m_selectedEntity.has<LightComponent>() == false)
                missingComponents.append("Light component");
            if (m_selectedEntity.has<MeshComponent>() == false)
                missingComponents.append("Mesh component");
            if (m_selectedEntity.has<ScriptComponent>() == false)
                missingComponents.append("Script component");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Add component"))
                ImGui::OpenPopup("ADD_COMP_POPUP");

            if (ImGui::BeginPopup("ADD_COMP_POPUP"))
            {
                for (auto comp : missingComponents)
                {
                    if (ImGui::Selectable((char*)comp))
                    {
                        if (comp == utils::String("Transform component"))
                            m_selectedEntity.emplace<TransformComponent>(math::vec3f{0,0,0},math::vec3f{0,0,0},math::vec3f{1,1,1});
                        if (comp == utils::String("Camera component"))
                            m_selectedEntity.emplace<GE::CameraComponent>((float)(60 * (PI / 180.0F)), 10000.0f, 0.01f);
                        if (comp == utils::String("Light component"))
                            m_selectedEntity.emplace<GE::LightComponent>(GE::LightComponent::Type::point, WHITE3, 1.0f);
                        if (comp == utils::String("Mesh component"))
                            m_selectedEntity.emplace<GE::MeshComponent>();
                    }
                }
                ImGui::EndPopup();
            }

            ImGui::PopItemWidth();
        }
    }
    ImGui::End();
}

void Editor::drawFPSPanel()
{
    if (ImGui::Begin("FPS"))
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::End();
}

void Editor::drawScenePickerPanel()
{
    if (ImGui::Begin("Scenes"))
    {
        float lineWith = 0.0F;
        for (auto& [name, scene] : m_project.game().scenes())
        {
            ImGui::Button((const char*)name, ImVec2(120, 120));

            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
            {
                const void* data = &scene;
                ImGui::SetDragDropPayload("SCENE_DND", &data, sizeof(Scene*));
                ImGui::Text("%s", (const char*)name);
                ImGui::EndDragDropSource();
            }

            lineWith += 127.5F;
            if (lineWith < ImGui::GetContentRegionAvail().x - 120.0f)
                ImGui::SameLine();
            else
                lineWith = 0.0F;
        }
        
        if (ImGui::Button("+", ImVec2(120, 120)))
        {
            m_newSceneName = "new scene";
            ImGui::OpenPopup("NEW_SCENE_MODAL_POPUP");
        }

        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f,0.5f));
        ImGui::SetNextWindowSize(ImVec2(0,0));
        if (ImGui::BeginPopupModal("NEW_SCENE_MODAL_POPUP", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove))
        {
            char buff[32];
            std::strncpy(buff, m_newSceneName, sizeof(buff));
            ImGui::InputText("Name", buff, sizeof(buff));
            m_newSceneName = utils::String(buff);

            if (ImGui::Button("Cancel"))
                ImGui::CloseCurrentPopup();

            ImGui::SameLine();

            if (ImGui::Button("OK"))
            {
                m_project.game().addScene(buff, Scene());
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }
    ImGui::End();
}

void Editor::drawSceneMeshPickerPanel()
{
    if (ImGui::Begin("Meshes"))
    {
        float lineWith = 0.0F;
        for (auto& [uuid, name] : m_editedScene->assetManager().registeredMesh())
        {
            ImGui::Button((const char*)name, ImVec2(120, 120));

            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
            {
                ImGui::SetDragDropPayload("MESH_DND", &uuid, sizeof(xg::Guid));
                ImGui::Text("%s", (const char*)name);
                ImGui::EndDragDropSource();
            }

            lineWith += 127.5F;
            if (lineWith < ImGui::GetContentRegionAvail().x - 120.0f)
                ImGui::SameLine();
            else
                lineWith = 0.0F;
        }
    }
    ImGui::End();
}

}
#endif