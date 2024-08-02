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
#include "Game-Engine/ECSWorld.hpp"
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
            ECSView<>(*m_runningGame->m_activeECSWorld)([&](ECSWorld::EntityID id) {
                if (m_runningGame->m_activeECSWorld->hasComponents<DebugNameComponent>(id))
                {
                    DebugNameComponent& comp = m_runningGame->m_activeECSWorld->getComponent<DebugNameComponent>(id);
                    if (ImGui::Selectable(comp.name, m_selectedEntity == id))
                        m_selectedEntity = id;
                }
                else
                {
                    if (ImGui::Selectable((char*)utils::String::fromUInt(id), m_selectedEntity == id))
                        m_selectedEntity = id;
                }
            });
        }
        
        ImGui::SeparatorText("Components");
        {
            if (m_runningGame->m_defaultECSWorld.isValid(m_selectedEntity) == false)
                ImGui::Text("No entity slected");
            else
            {
                if (m_runningGame->m_defaultECSWorld.hasComponents<TransformComponent>(m_selectedEntity))
                {
                    if (ImGui::TreeNode("Transform component"))
                    {
                        TransformComponent& comp = m_runningGame->m_defaultECSWorld.getComponent<TransformComponent>(m_selectedEntity);
                        ImGui::DragFloat3("position", (float*)&comp.position, 0.1F);
                        ImGui::DragFloat3("rotation", (float*)&comp.rotation, 0.01F, -2*PI, 2*PI);
                        ImGui::DragFloat3("scale",    (float*)&comp.scale,    0.01F,  0.0F, 5.0F);
                        ImGui::TreePop();
                    }
                }   

                if (m_runningGame->m_defaultECSWorld.hasComponents<ViewPointComponent>(m_selectedEntity))
                {
                    if (ImGui::TreeNode("View point component"))
                    {
                        ViewPointComponent& comp = m_runningGame->m_defaultECSWorld.getComponent<ViewPointComponent>(m_selectedEntity);
                        ImGui::DragFloat("fov",   (float*)&comp.fov,   0.01F, 0.0F, 180.0F);
                        ImGui::DragFloat("zNear", (float*)&comp.zNear, 0.01F, 0.0F, 10000.0F);
                        ImGui::DragFloat("zFar",  (float*)&comp.zFar,  1.0F,  0.0F, 10000.0F);
                        ImGui::TreePop();
                    }
                }   

                if (m_runningGame->m_defaultECSWorld.hasComponents<LightSourceComponent>(m_selectedEntity))
                {
                    if (ImGui::TreeNode("Light source component"))
                    {
                        LightSourceComponent& comp = m_runningGame->m_defaultECSWorld.getComponent<LightSourceComponent>(m_selectedEntity);
                        ImGui::ColorEdit3("color",    (float*)&comp.color);
                        ImGui::DragFloat("intensity", (float*)&comp.intensity, 0.001F,  0.0F, 1.0F);
                        ImGui::TreePop();
                    }
                }   
            }
        }
    }
    ImGui::End();

}

}