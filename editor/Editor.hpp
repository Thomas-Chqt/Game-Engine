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
#include "ECS/Entity.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "Graphics/FrameBuffer.hpp"
#include "UtilsCPP/Types.hpp"

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
    void resetEditorInputs();
    void editScene(Scene*);

    Project m_project;
    
    utils::SharedPtr<gfx::FrameBuffer> m_viewportFBuff;
    utils::uint32 m_viewportFBuffW = 800;
    utils::uint32 m_viewportFBuffH = 600;
    bool m_viewportFBuffSizeIsDirty = true;
    
    InputContext m_editorInputContext;
    EditorCamera m_editorCamera;

    Scene* m_editedScene = nullptr;
    Entity m_selectedEntity;

public:
    Editor& operator = (const Editor&) = delete;
    Editor& operator = (Editor&&)      = delete;
};

}

#endif // EDITOR_HPP