/*
 * ---------------------------------------------------
 * EditorUI.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/10 12:54:44
 * ---------------------------------------------------
 */

#include "ECS/Components.hpp"
#include "ECS/ECSView.hpp"
#include "ECS/Entity.hpp"
#include "Editor.hpp"
#include "Math/Constants.hpp"
#include "Scene.hpp"
#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/String.hpp"
#include "UtilsCPP/Types.hpp"
#include <TFD/tinyfiledialogs.h>
#include <cassert>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <future>
#include <string>

namespace GE
{

struct OpenProjectFileDialog
{
    bool isPresented = false;
    std::future<char*> result;

    void present()
    {
        isPresented = true;
        result = std::async(tinyfd_openFileDialog, "Open project", "", 0, nullptr, nullptr, 0);
    }
};

struct SaveProjectFileDialog
{
    bool isPresented = false;
    std::future<char*> result;

    void present(const utils::String& projectName)
    {
        isPresented = true;
        result = std::async(tinyfd_saveFileDialog, "Choose destination", projectName + ".geproj", 0, nullptr, nullptr);
    }
};

void Editor::onImGuiRender()
{
    static bool showProjectProperties = false;
    static bool showProjectScenes = false;
    static bool showDemoWindow = false;
    static OpenProjectFileDialog openProjectFileDialog;
    static SaveProjectFileDialog saveProjectFileDialog;

    ImGui::DockSpaceOverViewport();

    ImGui::BeginDisabled(openProjectFileDialog.isPresented || saveProjectFileDialog.isPresented);

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open"))
            {
                // if (char* path = tinyfd_openFileDialog("Open project", "", 0, nullptr, nullptr, 0))
                //     openProject(path);
                openProjectFileDialog.present();
            }

            if (ImGui::MenuItem("Save"))
            {
                if (m_projectFilePath.isEmpty())
                {
                    // if (char* path = tinyfd_saveFileDialog("Choose destination", m_projectName + ".geproj", 0, nullptr, nullptr))
                    // {
                    //     m_projectFilePath = path;
                    //     saveProject();
                    // }
                    saveProjectFileDialog.present(m_projectName);
                }
                else
                    saveProject();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Project"))
        {
            if (ImGui::MenuItem("Properties"))
                showProjectProperties = true;
            if (ImGui::MenuItem("Scene"))
                showProjectScenes = true;

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug"))
        {
            if (ImGui::MenuItem("Show demo window"))
                showDemoWindow = true;

            ImGui::EndMenu();
        }
            
        ImGui::EndMainMenuBar();
    }

    if (ImGui::Begin("viewport"))
    {
        ImVec2 contentRegionAvai = ImGui::GetContentRegionAvail();
        m_viewportPanelW = contentRegionAvai.x == 0 ? 1 : contentRegionAvai.x;
        m_viewportPanelH = contentRegionAvai.y == 0 ? 1 : contentRegionAvai.y;

        auto texture = m_viewportFBuff->colorTexture();
        ImGui::Image(texture->imguiTextureId(), contentRegionAvai, texture->imguiUV0(), texture->imguiUV1());
    }
    ImGui::End();

    if (ImGui::Begin("Scene graph"))
    {
        if (m_editedScene)
        {
            utils::Func<void(Entity)> renderRow = [&](Entity entity){
                bool node_open = false;
                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow        |
                                            ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                            ImGuiTreeNodeFlags_SpanAvailWidth    |
                                            ImGuiTreeNodeFlags_DefaultOpen;

                if (m_selectedEntity == entity)
                    flags |= ImGuiTreeNodeFlags_Selected;

                if (entity.has<HierarchyComponent>() == true && entity.firstChild())
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
                        renderRow(curr);
                    ImGui::TreePop();
                }
            };

            ECSView<NameComponent>(m_editedScene->ecsWorld()).onEach([&](Entity entity, NameComponent&) {
                if (entity.has<HierarchyComponent>() == false || entity.parent() == false)
                    renderRow(entity);
            });
        }
        else
        {
            ImGui::Text("No scene edited");
        }
    }
    ImGui::End();

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
            ImGui::InputText("Name##NameComponent", buff, sizeof(buff));
            nameComponent.name = utils::String(buff);

            if (m_selectedEntity.has<TransformComponent>())
            {
                bool isOpen = ImGui::CollapsingHeader("Transform component", ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_DefaultOpen);
                ImGui::SameLine(0, 30);
                if (ImGui::Button("remove##TransformComponent"))
                    m_selectedEntity.remove<TransformComponent>();
                else if (isOpen)
                {
                    TransformComponent& transform = m_selectedEntity.get<TransformComponent>();
                    ImGui::DragFloat3("position", (float*)&transform.position, 0.01f, -1000.0f, 1000.0f);
                    ImGui::DragFloat3("rotation", (float*)&transform.rotation, 0.01f,    -2*PI,    2*PI);
                    ImGui::DragFloat3("scale",    (float*)&transform.scale,    0.01f,     0.0f,   10.0f);
                }
            }

            if (m_selectedEntity.has<CameraComponent>())
            {
                bool isOpen = ImGui::CollapsingHeader("Camera component", ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_DefaultOpen);
                ImGui::SameLine(0, 30);
                if (ImGui::Button("remove##CameraComponent"))
                    m_selectedEntity.remove<CameraComponent>();
                else if (isOpen)
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

            if (m_selectedEntity.has<LightComponent>())
            {
                bool isOpen = ImGui::CollapsingHeader("Light component", ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_DefaultOpen);
                ImGui::SameLine(0, 30);
                if (ImGui::Button("remove##LightComponent"))
                    m_selectedEntity.remove<LightComponent>();
                else if (isOpen)
                {
                    LightComponent& lightComponent = m_selectedEntity.get<LightComponent>();
                    ImGui::ColorEdit3("color", (float*)&lightComponent.color);
                    ImGui::DragFloat("intentsity", &lightComponent.intentsity, 0.01, 0.0f, 1.0f);
                }
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Add component"))
                ImGui::OpenPopup("ADD_COMP_POPUP");

            if (ImGui::BeginPopup("ADD_COMP_POPUP"))
            {
                if (m_selectedEntity.has<TransformComponent>() == false && ImGui::Selectable("Transform component"))
                    m_selectedEntity.emplace<TransformComponent>(math::vec3f{0, 0, 0}, math::vec3f{0, 0, 0}, math::vec3f{1, 1, 1});

                if (m_selectedEntity.has<CameraComponent>() == false && ImGui::Selectable("Camera component"))
                    m_selectedEntity.emplace<CameraComponent>((float)(60 * (PI / 180.0F)), 10000.0f, 0.01f);

                if (m_selectedEntity.has<LightComponent>() == false && ImGui::Selectable("Light component"))
                    m_selectedEntity.emplace<LightComponent>(LightComponent::Type::point, WHITE3, 1.0f);

                ImGui::EndPopup();
            }
        }
        ImGui::PopItemWidth();
    }
    ImGui::End();

    if (ImGui::Begin("File explorer"))
    {
        assert(m_fileExplorerPath.is_absolute());
        assert(std::filesystem::is_directory(m_fileExplorerPath));

        std::filesystem::path curr;
        
        auto pathIt = m_fileExplorerPath.begin();
        curr /= *pathIt;
        if (ImGui::Button(pathIt->c_str()))
            m_fileExplorerPath = curr;

        ++pathIt;
        for (; pathIt != m_fileExplorerPath.end(); ++pathIt) {
            curr /= *pathIt;
            ImGui::SameLine();
            if (ImGui::Button(pathIt->c_str()))
                m_fileExplorerPath = curr;
        }

        if (ImGui::BeginChild("file_explorer_files"))
        {
            float lineWith = 0.0F;
            for (auto const& dir_entry : std::filesystem::directory_iterator(m_fileExplorerPath)) 
            {
                if (ImGui::BeginChild(dir_entry.path().filename().c_str(), ImVec2(60, 60 + ImGui::GetFrameHeightWithSpacing())))
                {
                    std::string id = dir_entry.path().string();
                    std::string extension = dir_entry.path().filename().extension();
                    std::string fileName = dir_entry.path().filename();

                    if (dir_entry.is_directory())
                    {
                        ImGui::Button(("dir##" + id).c_str(), ImVec2(60, 60));
                        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
                            m_fileExplorerPath = dir_entry.path();
                    }
                    else
                    {
                        if (dir_entry.path().has_extension())
                            ImGui::Button((extension + "##" + id).c_str(), ImVec2(60, 60));    
                        else
                            ImGui::Button((fileName + "##" + id).c_str(), ImVec2(60, 60));

                        if (ImGui::BeginDragDropSource())
                        {
                            ImGui::SetDragDropPayload("dnd_file", dir_entry.path().c_str(), strlen(dir_entry.path().c_str()));
                            ImGui::Text("%s", dir_entry.path().c_str());
                            ImGui::EndDragDropSource();
                        }
                    }

                    ImGui::Text("%s", dir_entry.path().filename().c_str());
                }
                ImGui::EndChild();

                lineWith += 67.5F;
                if (lineWith < ImGui::GetContentRegionAvail().x - 60.0f)
                    ImGui::SameLine();
                else
                    lineWith = 0.0F;
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();

    if (showProjectProperties)
        ImGui::OpenPopup("Project properties");
    if (ImGui::BeginPopupModal("Project properties", &showProjectProperties, ImGuiWindowFlags_AlwaysAutoResize))
    {
        char projectNameBuff[32];
        std::strncpy(projectNameBuff, m_projectName, sizeof(projectNameBuff));
        ImGui::InputText("Name##ProjectName", projectNameBuff, sizeof(projectNameBuff));
        m_projectName = utils::String(projectNameBuff);

        char resDirBuff[1024];
        std::strncpy(resDirBuff, m_projectRessourcesDir, sizeof(resDirBuff));
        ImGui::InputText("Ressource directory##ProjectRessourceDir", resDirBuff, sizeof(resDirBuff));
        m_projectRessourcesDir = utils::String(resDirBuff);

        ImGui::EndPopup();
    }

    if (showProjectScenes)
        ImGui::OpenPopup("Scenes");
    if (ImGui::BeginPopupModal("Scenes", &showProjectScenes, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Start scene: %s", (char*)utils::String(m_startScene ? m_startScene->name() : "None"));

        if (ImGui::Button("Create new scene"))
        {
            utils::String name = "new_scene";
            if (m_scenes.contain(name))
            {
                utils::uint32 i = 1;
                do {
                    name = "new_scene" + utils::String::fromUInt(i++);
                } while (m_scenes.contain(name));
            }
            m_scenes.insert(Scene(name));
        }
        
        ImGui::Spacing();
        
        const ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit;
        if (ImGui::BeginTable("table_scrolly", 2, flags, ImVec2(0, ImGui::GetFrameHeightWithSpacing() * (m_scenes.size() > 15 ? 15 : m_scenes.size()))))
        {
            ImGui::TableSetupColumn("1", ImGuiTableColumnFlags_WidthFixed, 150.0f);
            ImGui::TableSetupColumn("2");

            for (auto& scene : m_scenes)
            {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::PushItemWidth(-FLT_MIN);

                char buff[32];
                std::strncpy(buff, scene.name(), sizeof(buff));
                ImGui::InputText("##SceneName" + utils::String::fromUInt((utils::uint32)(utils::uint64)&scene), buff, sizeof(buff));
                scene.setName(utils::String(buff));

                ImGui::TableSetColumnIndex(1);
                ImGui::PushItemWidth(-FLT_MIN);

                if (ImGui::Button("Edit##" + scene.name()))
                {
                    editScene(&scene);
                    showProjectScenes = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Delete##" + scene.name()) && &scene != m_editedScene)
                {
                    m_scenes.remove(m_scenes.find(scene.name()));
                    break;
                }
            }

            ImGui::EndTable();
        }

        ImGui::EndPopup();
    }

    if (showDemoWindow)
        ImGui::ShowDemoWindow(&showDemoWindow);

    ImGui::EndDisabled();

    if (openProjectFileDialog.isPresented)
    {
        if (openProjectFileDialog.result.wait_for(std::chrono::nanoseconds(1)) == std::future_status::ready)
        {
            if (char* path = openProjectFileDialog.result.get())
                openProject(path);
            openProjectFileDialog.isPresented = false;
        }
            
    }
    if (saveProjectFileDialog.isPresented)
    {
        if (saveProjectFileDialog.result.wait_for(std::chrono::nanoseconds(1)) == std::future_status::ready)
        {
            if (char* path = saveProjectFileDialog.result.get())
            {
                m_projectFilePath = path;
                saveProject();

            }
            saveProjectFileDialog.isPresented = false;
        }
            
    }
}

}