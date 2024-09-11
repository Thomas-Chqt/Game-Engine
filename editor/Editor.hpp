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
#include "InputManager/InputContext.hpp"
#include "EditorCamera.hpp"
#include "Project.hpp"
#include "Scene.hpp"
#include "UtilsCPP/String.hpp"

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

    // imgui panels
    void drawViewportPanel();
    void drawSceneGraphPanel();
    void drawEntityInspectorPanel();
    void drawFPSPanel();
    void drawScenePickerPanel();
    void drawSceneMeshPickerPanel();

    void updateVPFrameBuff();
    void resetEditorInputs();
    void editScene(Scene*);

    Project m_project;
    Scene* m_editedScene = nullptr;

    InputContext m_editorInputContext;
    EditorCamera m_editorCamera;
    
    Entity m_selectedEntity;
    utils::String m_newSceneName; // TODO move to ScenePickerPanel struct

    utils::SharedPtr<gfx::FrameBuffer> m_viewportFBuff;
    math::vec2f m_viewportPanelSize = {800, 600};
    bool m_viewportPanelSizeIsDirty = true;

public:
    Editor& operator = (const Editor&) = delete;
    Editor& operator = (Editor&&)      = delete;
};

}

#endif // EDITOR_HPP