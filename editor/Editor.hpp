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
#include "Game.hpp"
#include "InputManager/InputContext.hpp"
#include "Project.hpp"
#include "Scene.hpp"
#include "ViewportFrameBuff.hpp"
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

    void runProject();
    void stopProject();

    void editScene(Scene*);

    void reloadScriptLib();
    
    ~Editor() = default;

private:
    void onUpdate() override;
    void onImGuiRender() override;
    void onWindowResizeEvent(gfx::WindowResizeEvent&) override;
    void onWindowRequestCloseEvent(gfx::WindowRequestCloseEvent&) override;

    void udpateEditorDatas();
    void processDroppedFiles();

    Project m_project;
    std::filesystem::path m_projectSavePath;

    Scene* m_editedScene = nullptr;
    Entity m_selectedEntity;
    EditorCamera m_editorCamera;
    InputContext m_editorInputContext;

    Game m_game;

    ViewportFrameBuff m_vpFrameBuff;
    bool m_imguiSettingsNeedReload = false;
    void* m_scriptLibHandle = nullptr;

public:
    Editor& operator = (const Editor&) = delete;
    Editor& operator = (Editor&&)      = delete;
};

}

#endif // EDITOR_HPP