/*
 * ---------------------------------------------------
 * Editor.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/02 16:04:23
 * ---------------------------------------------------
 */

#ifndef EDITOR_HPP
#define EDITOR_HPP

#include "Application.hpp"
#include "ECS/Entity.hpp"
#include "EditorCamera.hpp"
#include "InputManager/InputContext.hpp"
#include "Project.hpp"
#include "Scene.hpp"
#include "UI/EditorUI.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "Graphics/FrameBuffer.hpp"
#include <filesystem>

namespace GE
{

class Editor final : public Application
{
public:
    Editor();
    Editor(const Editor&) = delete;
    Editor(Editor&&)      = delete;

    void newProject();
    void openProject(const std::filesystem::path&);
    void reloadProject();
    void saveProject();

    void editScene(Scene*);

    void reloadScriptLib();
    
    ~Editor() = default;

private:
    void onUpdate() override;
    void onImGuiRender() override;
    void onEvent(gfx::Event&) override;

    void updateVPFrameBuff();
    void udpateEditorDatas();

    Project m_project;
    EditorUI m_ui;

    Scene* m_editedScene = nullptr;
    Entity m_selectedEntity;
    EditorCamera m_editorCamera;
    InputContext m_editorInputContext;

    bool m_imguiSettingsNeedReload = false;
    utils::SharedPtr<gfx::FrameBuffer> m_viewportFBuff;
    void* m_scriptLibHandle = nullptr;

public:
    Editor& operator = (const Editor&) = delete;
    Editor& operator = (Editor&&)      = delete;
};

}

#endif // EDITOR_HPP