/*
 * ---------------------------------------------------
 * SceneGraphPanel.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/13 15:52:17
 * ---------------------------------------------------
 */

#include "imgui/SceneGraphPanel.hpp"

namespace GE
{

SceneGraphPanel::SceneGraphPanel(Scene* scene, Entity selectedEntity)
    : m_scene(scene),
      m_selectedEntity(selectedEntity)
{
}

SceneGraphPanel& SceneGraphPanel::onEntitySelect(const utils::Func<void(Entity)>& func)
{
    m_onEntitySelect = func;
    return *this;
}

void SceneGraphPanel::render()
{
    if (ImGui::Begin("Scene graph"))
    {
        if (m_scene)
        {
            m_scene->forEachNamedEntity([&](Entity entity, NameComponent&) {
                if (entity.has<HierarchicalComponent>() == false || entity.parent() == false)
                    renderRow(entity);
            });
        }
        else
        {
            ImGui::Text("No scene edited");
        }
    }
    ImGui::End();
}

void SceneGraphPanel::renderRow(Entity entity)
{
    bool node_open = false;
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow        |
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

    if (ImGui::IsItemClicked() && m_onEntitySelect)
        m_onEntitySelect(entity);

    if (node_open)
    {
        for (Entity curr = entity.firstChild(); curr; curr = curr.nextChild() )
            renderRow(curr);
        ImGui::TreePop();
    }
}

}