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
#include "Script.hpp"

namespace GE
{

class Editor final : public Application
{
public:
    Editor();
    Editor(const Editor&) = delete;
    Editor(Editor&&)      = delete;

    void onUpdate() override;
    void onImGuiRender() override;
    inline void onWindowResizeEvent(gfx::WindowResizeEvent&) override {}
    void onWindowRequestCloseEvent(gfx::WindowRequestCloseEvent&) override;

    // * Functions that directly match user action
    // * Some are also used by other functions
    void newProject();
    void openProject(const std::filesystem::path&);
    void reloadProject();
    void saveProject();
    void reloadScriptLib();
    void editScene(Scene*);
    void runGame();
    void stopGame();

    ~Editor();

private:
    // * helper functions
    void processDroppedFiles();

    // * absolut path of the project file 
    // * (can be empty if the project hasnt been saved yet)
    std::filesystem::path m_projectSavePath;

    // * project data (saved to disk)
    Project m_project;

    // * data derived from the project file (created at runtime, not saved)
    void* m_scriptLibHandle = nullptr;
    GetScriptNamesFn m_getScriptNames = nullptr;
    MakeScriptInstanceFn m_makeScriptInstance = nullptr;

    // * editor state
    // TODO save this data in the project so the last state can be preserved
    Scene* m_editedScene = nullptr;
    Entity m_selectedEntity;
    EditorCamera m_editorCamera;
    InputContext m_editorInputContext;

    // * instance used when the game is running
    utils::UniquePtr<Game> m_game;

    // * UI reladed data
    ViewportFrameBuff m_vpFrameBuff;

public:
    Editor& operator = (const Editor&) = delete;
    Editor& operator = (Editor&&)      = delete;
};

}

#endif // EDITOR_HPP