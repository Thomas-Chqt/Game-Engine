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
#include "Project.hpp"
#include "Scene.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "Graphics/FrameBuffer.hpp"
#include "UtilsCPP/String.hpp"
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
    struct UIStates
    {
        bool showDemoWindow = false;

        bool showNewProjectPopupModal = false; 
        utils::String newProjectName;
        utils::String newProjectPath;

        bool showProjectProperties = false;
        bool imguiSettingsNeedReload = false;
    };

    void onUpdate() override;
    void onImGuiRender() override;
    void onEvent(gfx::Event&) override;

    void newProject(const utils::String& name, const utils::String& path);
    void openProject(const utils::String& filePath);
    void saveProject();

    void newScene();
    void editScene(Scene*);

    void updateVPFrameBuff();
    void resetEditorInputs();

    utils::String m_projFilePath;
    Project m_project;
        
    utils::SharedPtr<gfx::FrameBuffer> m_viewportFBuff;
    utils::uint32 m_viewportFBuffW = 800;
    utils::uint32 m_viewportFBuffH = 600;
    bool m_viewportFBuffSizeIsDirty = true;

    UIStates m_uiStates;

public:
    Editor& operator = (const Editor&) = delete;
    Editor& operator = (Editor&&)      = delete;
};

}

#endif // EDITOR_HPP