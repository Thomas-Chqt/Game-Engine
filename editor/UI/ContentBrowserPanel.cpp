/*
 * ---------------------------------------------------
 * ContentBrowserPanel.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/11/06 14:48:45
 * ---------------------------------------------------
 */

#include "UI/ContentBrowserPanel.hpp"
#include "Script.hpp"
#include <cstring>

#define ELEMENT_SIZE 60.0F

namespace GE
{

ContentBrowserPanel::ContentBrowserPanel(Project& project, const Scene* scene, GetScriptNamesFn getScriptNames)
    : m_project(project), m_scene(scene), m_getScriptNames(getScriptNames)
{
}

void ContentBrowserPanel::render()
{
    if (ImGui::Begin("Content browser"))
    {
        renderScenes();
        renderAssets();
        renderScripts();
    }
    ImGui::End();
}

void ContentBrowserPanel::renderElement(const utils::String& name, const char *payloadType, const void* payload, utils::uint64 payloadSize)
{
    if (ImGui::BeginChild(name + "childId", ImVec2(ELEMENT_SIZE, ELEMENT_SIZE + ImGui::GetFrameHeightWithSpacing())))
    {
        ImGui::Button(name, ImVec2(ELEMENT_SIZE, ELEMENT_SIZE));
        if (ImGui::BeginDragDropSource())
        {
            ImGui::SetDragDropPayload(payloadType, payload, payloadSize);
            ImGui::Text("%s", (const char*)name);
            ImGui::EndDragDropSource();
        }
        ImGui::Text("%s", (const char*)name);
    }
    ImGui::EndChild();

    m_lineWith += ELEMENT_SIZE + 7.5F;
    if (m_lineWith < ImGui::GetContentRegionAvail().x - ELEMENT_SIZE)
        ImGui::SameLine();
    else
        m_lineWith = 0.0F;

}

void ContentBrowserPanel::renderScenes()
{
    for (auto& scene : m_project.scenes())
        renderElement(scene.name(), "scene_dnd", (void*)(const char*)scene.name(), scene.name().length());
}

void ContentBrowserPanel::renderAssets()
{
    if (m_scene == nullptr)
        return;

    for (auto& [path, id] : m_scene->assetManager().registeredMeshes())
        renderElement(path.filename().string().c_str(), "mesh_dnd", &id, sizeof(id));
}

void ContentBrowserPanel::renderScripts()
{
    if (m_getScriptNames == nullptr)
        return;

    const char** scriptNames;
    utils::uint64 scriptCount;
    m_getScriptNames(&scriptNames, &scriptCount);

    for (utils::uint64 i = 0; i < scriptCount; i++)
        renderElement(scriptNames[i], "script_dnd", scriptNames[i], strlen(scriptNames[i]));
}


}