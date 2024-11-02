/*
 * ---------------------------------------------------
 * Editor.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/03 14:42:56
 * ---------------------------------------------------
 */

#include "Editor.hpp"
#include "ECS/Entity.hpp"
#include "EditorCamera.hpp"
#include "Graphics/Event.hpp"
#include "Graphics/RenderTarget.hpp"
#include "InputManager/RawInput.hpp"
#include "InputManager/Mapper.hpp"
#include "Project.hpp"
#include "Scene.hpp"
#include "UI/MainMenuBar.hpp"
#include "UI/ProjectPropertiesModal.hpp"
#include "UI/ViewportPanel.hpp"
#include "UI/SceneGraphPanel.hpp"
#include "UtilsCPP/String.hpp"
#include "UtilsCPP/Types.hpp"
#include "UtilsCPP/UniquePtr.hpp"
#include <cassert>
#include <dlfcn.h>
#include <filesystem>
#include <nlohmann/json.hpp>
#include "imgui.h"

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace GE
{

Editor::Editor()
{
    ImGui::GetIO().IniFilename = nullptr;

    Mapper<KeyboardButton, Range2DInput>::Descriptor inputMapperDesc;

    ActionInput& quitEditorIpt = m_editorInputContext.newInput<ActionInput>("quit_editor");
    quitEditorIpt.callback = utils::Func<void()>(*(Application*)this, &Application::terminate);
    auto quitEditorIptMapper = utils::makeUnique<Mapper<KeyboardButton, ActionInput>>(KeyboardButton::esc, quitEditorIpt);
    quitEditorIpt.mappers[0] = quitEditorIptMapper.staticCast<IMapper>();
    quitEditorIpt.mappers[1].clear();

    Range2DInput& editorCamMoveIpt = m_editorInputContext.newInput<Range2DInput>("editor_cam_move");
    editorCamMoveIpt.callback = utils::Func<void(math::vec2f)>(m_editorCamera, &EditorCamera::move);
    inputMapperDesc.xPos = KeyboardButton::d;
    inputMapperDesc.xNeg = KeyboardButton::a;
    inputMapperDesc.yPos = KeyboardButton::w;
    inputMapperDesc.yNeg = KeyboardButton::s;
    auto editorCamMoveIptMapper = utils::makeUnique<Mapper<KeyboardButton, Range2DInput>>(inputMapperDesc, editorCamMoveIpt);
    editorCamMoveIpt.mappers[0] = editorCamMoveIptMapper.staticCast<IMapper>();
    editorCamMoveIpt.mappers[1].clear();

    Range2DInput& editorCamRotateIpt = m_editorInputContext.newInput<Range2DInput>("editor_cam_rotate");
    editorCamRotateIpt.callback = utils::Func<void(math::vec2f)>(m_editorCamera, &EditorCamera::rotate);
    inputMapperDesc.xPos = KeyboardButton::down;
    inputMapperDesc.xNeg = KeyboardButton::up;
    inputMapperDesc.yPos = KeyboardButton::right;
    inputMapperDesc.yNeg = KeyboardButton::left;
    auto editorCamRotateIptMapper = utils::makeUnique<Mapper<KeyboardButton, Range2DInput>>(inputMapperDesc, editorCamRotateIpt);
    editorCamRotateIpt.mappers[0] = editorCamRotateIptMapper.staticCast<IMapper>();
    editorCamRotateIpt.mappers[1].clear();

    newProject();
}

void Editor::newProject()
{
    m_project = Project();
    udpateEditorDatas();
}

void Editor::openProject(const fs::path& filePath)
{
    m_project = Project();
    udpateEditorDatas();
}

void Editor::reloadProject()
{
    m_project.reload();
    udpateEditorDatas();
}

void Editor::saveProject()
{
    m_project.save();
}

void Editor::editScene(Scene* scene)
{
    if (m_editedScene && m_editedScene->isLoaded())
        m_editedScene->unload();
    
    if (m_project.ressourcesDir().empty())
        scene->load(m_renderer.graphicAPI(), fs::path());
    else
        scene->load(m_renderer.graphicAPI(), fs::path(m_project.savePath()).remove_filename() / m_project.ressourcesDir());
    
    m_editedScene = scene;
    m_selectedEntity = Entity();
    m_editorCamera = EditorCamera();
}

void Editor::reloadScriptLib()
{
    if (m_scriptLibHandle != nullptr)
    {
        dlclose(m_scriptLibHandle);
        m_scriptLibHandle = nullptr;
    }

    fs::path absoluteScriptLibPath;
    if (m_project.scriptLib().is_absolute())
        absoluteScriptLibPath = m_project.scriptLib();
    else if (m_project.hasSavePath())
        absoluteScriptLibPath = fs::path(m_project.savePath()).remove_filename() / m_project.scriptLib();

    if (m_project.scriptLib().empty() == false)
    {
        fs::path absoluteScriptLibPath = fs::path(m_project.savePath()).remove_filename() / m_project.scriptLib();
        if (fs::is_regular_file(absoluteScriptLibPath))
        {
            m_scriptLibHandle = dlopen(absoluteScriptLibPath.c_str(), RTLD_NOW | RTLD_GLOBAL);
            assert(m_scriptLibHandle != nullptr);       
        }
    }
}

void Editor::onUpdate()
{
    updateVPFrameBuff();

    m_renderer.beginScene(m_editorCamera.getRendererCam(), m_viewportFBuff.staticCast<gfx::RenderTarget>());
    {
        if (m_editedScene)
            m_renderer.addScene(*m_editedScene);
    }
    m_renderer.endScene();

    if (ImGui::GetIO().WantCaptureKeyboard == false)
        m_editorInputContext.dispatchInputs();
    else
        m_editorInputContext.resetInputs();
}

void Editor::onImGuiRender()
{
    if (m_imguiSettingsNeedReload)
    {
        m_project.loadIniSettingsFromMemory();
        m_imguiSettingsNeedReload = false;
    }

    static bool isProjectPropertiesModalPresented = false;

    ImGui::DockSpaceOverViewport();

    MainMenuBar()
        .on_File_New([](){})
        .on_File_Open([](){})
        .on_File_Save([](){})
        .on_Project_Properties([](){ isProjectPropertiesModalPresented = true; })
        .on_Project_Scene(utils::Func<void()>())
        .on_Project_Run(/*project not running*/0 ? [](){} : utils::Func<void()>())
        .on_Project_Stop(/*project running*/0 ? [](){} : utils::Func<void()>())
        .render();

    ViewportPanel(*m_viewportFBuff->colorTexture())
        .onResize([&](utils::uint32 w, utils::uint32 h){ m_viewportPanelW = w; m_viewportPanelH = h; })
        .render();

    ProjectPropertiesModal(isProjectPropertiesModalPresented, m_project)
        .render();

    SceneGraphPanel(m_editedScene, m_selectedEntity)
        .onEntitySelect([&](const Entity& e){ m_selectedEntity = e; })
        .render();

    if (ImGui::GetIO().WantSaveIniSettings)
    {
        m_project.saveIniSettingsToMemory();
        ImGui::GetIO().WantSaveIniSettings = false;
    } 
}

void Editor::onEvent(gfx::Event& event)
{
    if (event.dispatch(utils::Func<void(gfx::InputEvent&)>(m_editorInputContext, &InputContext::onInputEvent)))
        return;
    if (event.dispatch<gfx::WindowResizeEvent>([&](gfx::WindowResizeEvent& windowResizeEvent) {
        //
    })) return;
    if (event.dispatch<gfx::WindowRequestCloseEvent>([&](gfx::WindowRequestCloseEvent& windowRequestCloseEvent) {
        terminate();
    })) return;
}

void Editor::updateVPFrameBuff()
{
    float xScale, yScale;
    m_window->getFrameBufferScaleFactor(&xScale, &yScale);

    utils::uint32 newFrameBufferWidth = (utils::uint32)((float)m_viewportPanelW * xScale);
    utils::uint32 newFrameBufferHeight = (utils::uint32)((float)m_viewportPanelH * yScale);

    if (m_viewportFBuff && (m_viewportFBuff->width() == newFrameBufferWidth && m_viewportFBuff->height() == newFrameBufferHeight))
        return;
    
    gfx::Texture::Descriptor colorTextureDescriptor;
    colorTextureDescriptor.width = newFrameBufferWidth;
    colorTextureDescriptor.height = newFrameBufferHeight;
    colorTextureDescriptor.pixelFormat = gfx::PixelFormat::BGRA;
    colorTextureDescriptor.usage = gfx::Texture::Usage::ShaderReadAndRenderTarget;
    
    gfx::Texture::Descriptor depthTextureDescriptor = gfx::Texture::Descriptor::depthTextureDescriptor(newFrameBufferWidth, newFrameBufferHeight);

    gfx::FrameBuffer::Descriptor fBuffDesc;
    fBuffDesc.colorTexture = m_renderer.graphicAPI().newTexture(colorTextureDescriptor);
    fBuffDesc.depthTexture = m_renderer.graphicAPI().newTexture(depthTextureDescriptor);
    m_viewportFBuff = m_renderer.graphicAPI().newFrameBuffer(fBuffDesc);
}

void Editor::udpateEditorDatas()
{
    editScene(m_project.startScene());

    // if (m_project.ressourcesDir().empty())
        // m_ui.setFileExplorerPath(std::filesystem::current_path());
    // else
        // m_ui.setFileExplorerPath(fs::path(m_project.savePath()).remove_filename() / m_project.ressourcesDir());

    reloadScriptLib();
}

}