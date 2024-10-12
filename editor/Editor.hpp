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
#include "EditorCamera.hpp"
#include "InputManager/InputContext.hpp"
#include "Scene.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "Graphics/FrameBuffer.hpp"
#include "UtilsCPP/String.hpp"
#include "UtilsCPP/Types.hpp"
#include <filesystem>

namespace GE
{

class Editor final : public Application
{
public:
    Editor();
    Editor(const Editor&) = delete;
    Editor(Editor&&)      = delete;
    
    ~Editor() = default;

private:
    void onUpdate() override;
    void onImGuiRender() override;
    void onEvent(gfx::Event&) override;

    void updateVPFrameBuff();

    void openProject(const utils::String& path);
    void saveProject();

    void editScene(Scene*);

    void resetEditorInputs();

    utils::String m_projectFilePath;
    utils::String m_projectName;
    utils::String m_projectRessourcesDir;

    utils::String m_imguiSettings;
    bool m_imguiSettingsNeedReload = false;

    utils::Set<Scene> m_scenes;
    Scene* m_startScene = nullptr;

    Scene* m_editedScene = nullptr;
    Entity m_selectedEntity;
    EditorCamera m_editorCamera;

    InputContext m_editorInputContext;

    utils::SharedPtr<gfx::FrameBuffer> m_viewportFBuff;

    utils::uint32 m_viewportPanelW = 800;
    utils::uint32 m_viewportPanelH = 600;
    std::filesystem::path m_fileExplorerPath;

public:
    Editor& operator = (const Editor&) = delete;
    Editor& operator = (Editor&&)      = delete;
};

}

#endif // EDITOR_HPP