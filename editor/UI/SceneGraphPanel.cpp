/*
 * ---------------------------------------------------
 * SceneGraphPanel.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/11/01 16:58:07
 * ---------------------------------------------------
 */

#include "UI/SceneGraphPanel.hpp"
#include "Scene.hpp"
#include "ECS/ECSView.hpp"
#include "ECS/Components.hpp"

namespace GE
{

SceneGraphPanel::SceneGraphPanel(Scene* scene, const Entity& selectedEntity)
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
                ECSView<NameComponent>(m_scene->ecsWorld()).onEach([&](Entity entity, NameComponent&) {
                    if (entity.hasParent() == false)
                        renderEntityRow(entity);
                });
            }
            else
            {
                ImGui::Text("No scene edited");
            }
            ImGui::EndChild();
        }
        entityDndTarget();
    }
    ImGui::End();
}

void SceneGraphPanel::renderEntityRow(Entity& entity)
{
        bool node_open = false;
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;

        if (m_selectedEntity == entity)
            flags |= ImGuiTreeNodeFlags_Selected;

        if (entity.childCount() > 0)
            node_open = ImGui::TreeNodeEx(entity.imGuiID(), flags, "%s", (const char*)entity.name());
        else
        {
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            ImGui::TreeNodeEx(entity.imGuiID(), flags, "%s", (const char*)entity.name());
        }

        if (ImGui::BeginDragDropSource())
        {
            ImGui::SetDragDropPayload("dnd_entity", &entity, sizeof(Entity));
            ImGui::Text("%s", (char*)entity.name());
            ImGui::EndDragDropSource();
        }

        entityDndTarget(entity);

        if (ImGui::IsItemClicked() && m_onEntitySelect)
            m_onEntitySelect(entity);

        if (node_open && entity.childCount() > 0)
        {
            for (Entity curr = entity.firstChild(); curr; curr = curr.nextChild() )
                renderEntityRow(curr);
            ImGui::TreePop();
        }

}

void SceneGraphPanel::entityDndTarget(Entity parent)
{
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("dnd_entity"))
        {
            assert(payload->DataSize == sizeof(Entity));
            Entity dragEntity = *(Entity*)payload->Data;
            dragEntity.removeParent();

            if (parent && parent != dragEntity && dragEntity.isParentOf(parent) == false)
            {
                parent.addChild(dragEntity);
            }
        }
        ImGui::EndDragDropTarget();
    }
}

}