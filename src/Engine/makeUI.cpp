/*
 * ---------------------------------------------------
 * makeUI.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/01 11:47:21
 * ---------------------------------------------------
 */

#include "Engine/InternalEngine.hpp"
#include "Game-Engine/Components.hpp"
#include "Math/Constants.hpp"
#include "UtilsCPP/String.hpp"

namespace GE
{

void InternalEngine::makeUI()
{
    if (ImGui::Begin("Imgui", nullptr, ImGuiWindowFlags_MenuBar))
    {
        if (ImGui::BeginMenuBar())
        {
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::EndMenuBar();
        }

        ImGui::SeparatorText("Entities");
        {
            for (auto entity : *m_runningGame->m_activeECSWorld)
            {
                if (entity.has<DebugNameComponent>())
                {
                    DebugNameComponent& comp = entity.get<DebugNameComponent>();
                    if (ImGui::Selectable(comp.name, m_selectedEntity == entity))
                        m_selectedEntity = entity;
                }
                else
                {
                    if (ImGui::Selectable((char*)utils::String::fromUInt(entity.id), m_selectedEntity == entity))
                        m_selectedEntity = entity;
                }
            }
        }
        
        ImGui::SeparatorText("Components");
        {
            if (m_runningGame->m_defaultECSWorld.isValid(m_selectedEntity) == false)
                ImGui::Text("No entity slected");
            else
            {
                if (m_selectedEntity.has<TransformComponent>())
                {
                    if (ImGui::TreeNode("Transform component"))
                    {
                        ImGui::DragFloat3("position", (float*)&m_selectedEntity.get<TransformComponent>().position, 0.1F);
                        ImGui::DragFloat3("rotation", (float*)&m_selectedEntity.get<TransformComponent>().rotation, 0.01F, -2*PI, 2*PI);
                        ImGui::DragFloat3("scale",    (float*)&m_selectedEntity.get<TransformComponent>().scale,    0.01F,  0.0F, 5.0F);
                        ImGui::TreePop();
                    }
                }   

                if (m_selectedEntity.has<ViewPointComponent>())
                {
                    if (ImGui::TreeNode("View point component"))
                    {
                        ImGui::DragFloat("fov",   (float*)&m_selectedEntity.get<ViewPointComponent>().fov,   0.01F, 0.0F, 180.0F);
                        ImGui::DragFloat("zNear", (float*)&m_selectedEntity.get<ViewPointComponent>().zNear, 0.01F, 0.0F, 10000.0F);
                        ImGui::DragFloat("zFar",  (float*)&m_selectedEntity.get<ViewPointComponent>().zFar,  1.0F,  0.0F, 10000.0F);
                        ImGui::TreePop();
                    }
                }   

                if (m_selectedEntity.has<LightSourceComponent>())
                {
                    if (ImGui::TreeNode("Light source component"))
                    {
                        ImGui::ColorEdit3("color",    (float*)&m_selectedEntity.get<LightSourceComponent>().color);
                        ImGui::DragFloat("intensity", (float*)&m_selectedEntity.get<LightSourceComponent>().intensity, 0.001F,  0.0F, 1.0F);
                        ImGui::TreePop();
                    }
                }   
            }
        }
    }
    ImGui::End();

}

}