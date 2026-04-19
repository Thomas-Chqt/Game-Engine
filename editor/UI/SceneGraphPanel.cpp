/*
 * ---------------------------------------------------
 * SceneGraphPanel.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/11/01 16:58:07
 * ---------------------------------------------------
 */

#include "UI/SceneGraphPanel.hpp"

#include <Game-Engine/Scene.hpp>
#include <Game-Engine/Entity.hpp>
#include <Game-Engine/ECSView.hpp>

#include <imgui.h>

namespace GE_Editor
{

SceneGraphPanel::SceneGraphPanel(GE::Scene* scene, GE::Entity* selectedEntity)
    : m_scene(scene), m_selectedEntity(selectedEntity)
{
}

void SceneGraphPanel::render()
{
    if (ImGui::Begin("Scene graph"))
    {
        if (ImGui::BeginChild("scene_graph_child"))
        {
            if (m_scene)
            {
                for (GE::Entity entity : m_scene->ecsWorld()
                                         | GE::ECSView<GE::NameComponent>()
                                         | std::views::transform([&](auto id){ return GE::Entity{&m_scene->ecsWorld(), id}; }))
                {
                    if (entity.parent().has_value() == false)
                        renderEntityRow(entity);
                }
            }
            else
            {
                ImGui::Text("No scene edited");
            }
            ImGui::EndChild();
        }
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("dnd_entity")) {
                assert(payload->DataSize == sizeof(GE::Entity));
                GE::Entity& droped = *(GE::Entity*)payload->Data;
                if(auto parent = droped.parent())
                    parent->removeChild(droped);
            }
            ImGui::EndDragDropTarget();
        }
    }
    ImGui::End();
}

void SceneGraphPanel::renderEntityRow(GE::Entity& entity)
{
    bool node_open = false;
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;

    if (m_selectedEntity && *m_selectedEntity == entity)
        flags |= ImGuiTreeNodeFlags_Selected;

    if (entity.children().size() > 0)
        node_open = ImGui::TreeNodeEx((void*)entity.entityId, flags, "%s", entity.name().c_str());
    else
    {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        ImGui::TreeNodeEx((void*)entity.entityId, flags, "%s", entity.name().c_str());
    }

    if (ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("dnd_entity", &entity, sizeof(GE::const_Entity));
        ImGui::Text("%s", entity.name().c_str());
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("dnd_entity")) {
            assert(payload->DataSize == sizeof(GE::Entity));
            GE::Entity& droped = *(GE::Entity*)payload->Data;
            if (droped.isParentOf(entity) == false) {
                if(auto parent = droped.parent())
                    parent->removeChild(droped);
                entity.addChild(droped);
            }
        }
        ImGui::EndDragDropTarget();
    }

    if (ImGui::IsItemClicked() && m_selectedEntity)
        *m_selectedEntity = entity;

    if (node_open && entity.children().size() > 0)
    {
        for (auto curr = entity.firstChild(); curr; curr = curr->nextChild() )
            renderEntityRow(*curr);
        ImGui::TreePop();
    }
}

}
