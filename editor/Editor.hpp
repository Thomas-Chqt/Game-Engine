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

    void drawViewportPanel();
    void drawSceneGraphPanel();
    void drawEntityInspectorPanel();
    void drawFPSPanel();

    void updateVPFrameBuff();

    void resetEditorInputs();

    Entity m_selectedEntity;

    utils::SharedPtr<gfx::FrameBuffer> m_viewportFBuff;
    math::vec2f m_viewportPanelSize = {800, 600};
    bool m_viewportPanelSizeIsDirty = true;

    InputContext m_editorInputContext;
    EditorCamera m_editorCamera;

public:
    Editor& operator = (const Editor&) = delete;
    Editor& operator = (Editor&&)      = delete;
};

}

#endif // EDITOR_HPP