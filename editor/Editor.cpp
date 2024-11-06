/*
 * ---------------------------------------------------
 * Editor.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/03 14:42:56
 * ---------------------------------------------------
 */

#include "Editor.hpp"
#include "ECS/Components.hpp"
#include "ECS/ECSView.hpp"
#include "ECS/Entity.hpp"
#include "EditorCamera.hpp"
#include "Graphics/Event.hpp"
#include "Graphics/RenderTarget.hpp"
#include "InputManager/RawInput.hpp"
#include "InputManager/Mapper.hpp"
#include "Project.hpp"
#include "Scene.hpp"
#include "UI/ContentBrowserPanel.hpp"
#include "UI/EntityInspectorPanel.hpp"
#include "UI/FileOpenDialog.hpp"
#include "UI/FileSaveDialog.hpp"
#include "UI/MainMenuBar.hpp"
#include "UI/ProjectPropertiesModal.hpp"
#include "UI/ViewportPanel.hpp"
#include "UI/SceneGraphPanel.hpp"
#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/String.hpp"
#include "UtilsCPP/Types.hpp"
#include "UtilsCPP/UniquePtr.hpp"
#include <cassert>
#include <dlfcn.h>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include "imgui.h"
#include <stb_image/stb_image.h>

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace GE
{

Editor::Editor()
{
    ImGui::GetIO().IniFilename = nullptr;
    // ImGui::GetIO().SetClipboardTextFn = [](void* user_data, const char* text) { static_cast<gfx::Window*>(user_data)->setClipboardString(text); };
    // ImGui::GetIO().GetClipboardTextFn = [](void* user_data) -> const char* { return static_cast<gfx::Window*>(user_data)->getClipboardString(); };
    // ImGui::GetIO().ClipboardUserData = (gfx::Window*)m_window;

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
    m_projectSavePath = fs::path();
    m_project = Project();
    udpateEditorDatas();
}

void Editor::openProject(const fs::path& filePath)
{
    assert(fs::is_regular_file(filePath));
    assert(filePath.is_absolute());
    
    m_projectSavePath = filePath;
    reloadProject();
}

void Editor::reloadProject()
{
    m_project = Project(m_projectSavePath);
    udpateEditorDatas();
}

void Editor::saveProject()
{
    m_project.save(m_projectSavePath);
}

void Editor::runProject()
{
    using makeScriptInstanceFunc = GE::Script* (*)(char*);

    m_runningScene = utils::makeUnique<Scene>(*m_editedScene);

    if (m_scriptLibHandle == nullptr)
        return;

    ECSView<ScriptComponent>(m_runningScene->ecsWorld()).onEach([&](Entity entt, ScriptComponent& scriptComponent) {
        auto makeScriptInstance = (makeScriptInstanceFunc)dlsym(m_scriptLibHandle, "makeScriptInstance");
        if (makeScriptInstance)
        {
            scriptComponent.instance = utils::SharedPtr<Script>(makeScriptInstance(scriptComponent.name));
            scriptComponent.instance->setEntity(entt);
        } 
    });
}

void Editor::stopProject()
{
    m_runningScene.clear();
}

void Editor::editScene(Scene* scene)
{
    if (m_editedScene && m_editedScene->isLoaded())
        m_editedScene->unload();
    
    scene->load(m_renderer.graphicAPI(), fs::path(m_projectSavePath).remove_filename());
    
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

    if (m_project.scriptLib().empty() == false)
    {
        fs::path scriptLibPath = fs::path(m_projectSavePath).remove_filename() / m_project.scriptLib();
        assert(scriptLibPath.is_absolute());
        if (fs::is_regular_file(scriptLibPath))
            m_scriptLibHandle = dlopen(scriptLibPath.c_str(), RTLD_NOW | RTLD_GLOBAL);
    }
}

void Editor::onUpdate()
{
    updateVPFrameBuff();
    processDroppedFiles();

    if (!m_runningScene)
    {
        m_renderer.beginScene(m_editorCamera.getRendererCam(), m_viewportFBuff.staticCast<gfx::RenderTarget>());
        {
            if (m_editedScene)
                m_renderer.addScene(*m_editedScene);
        }
        m_renderer.endScene();
    }
    else
    {
        ECSView<ScriptComponent>(m_runningScene->ecsWorld()).onEach([&](Entity entt, ScriptComponent& scriptComponent){
            if (scriptComponent.instance)
                scriptComponent.instance->onUpdate();
        });

        m_renderer.beginScene(m_editorCamera.getRendererCam(), m_viewportFBuff.staticCast<gfx::RenderTarget>());
        {
            m_renderer.addScene(*m_runningScene);
        }
        m_renderer.endScene();
    }

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
    static bool isFileOpenDialogPresented = false;
    static bool isFileSaveDialogPresented = false;

    ImGui::BeginDisabled(isFileOpenDialogPresented || isFileSaveDialogPresented);

    ImGui::DockSpaceOverViewport();

    MainMenuBar()
        .on_File_New(utils::Func<void()>(*this, &Editor::newProject))
        .on_File_Open([](){ isFileOpenDialogPresented = true; })
        .on_File_Save([&](){ m_projectSavePath.empty() ? (void)(isFileSaveDialogPresented = true) : saveProject(); })
        .on_Project_ReloadScriptLib(!m_project.scriptLib().empty() ? utils::Func<void()>(*this, &Editor::reloadScriptLib) : utils::Func<void()>())
        .on_Project_Properties([](){ isProjectPropertiesModalPresented = true; })
        .on_Project_Run(!m_runningScene ? utils::Func<void()>(*this, &Editor::runProject) : utils::Func<void()>())
        .on_Project_Stop(m_runningScene ? utils::Func<void()>(*this, &Editor::stopProject) : utils::Func<void()>())
        .render();

    ViewportPanel(*m_viewportFBuff->colorTexture())
        .onResize([&](utils::uint32 w, utils::uint32 h){
            m_viewportPanelW = w;
            m_viewportPanelH = h;
        })
        .render();

    SceneGraphPanel(m_editedScene, m_selectedEntity)
        .onEntitySelect([&](const Entity& e){ m_selectedEntity = e; })
        .render();

    EntityInspectorPanel(m_editedScene, m_selectedEntity)
        .onEntityDelete([&](){
            m_selectedEntity.destroy();
            m_selectedEntity = Entity();
        })
        .render();

    ProjectPropertiesModal(isProjectPropertiesModalPresented, m_project, m_projectSavePath)
        .onOk(utils::Func<void()>(*this, &Editor::reloadScriptLib))
        .render();

    ContentBrowserPanel(m_project, m_editedScene, m_scriptLibHandle)
        .render();

    ImGui::EndDisabled();

    FileOpenDialog("Open project", isFileOpenDialogPresented)
        .onSelection(utils::Func<void(const fs::path&)>(*this, &Editor::openProject))
        .render();

    FileSaveDialog(m_project.name() + ".geproj", isFileSaveDialogPresented)
        .onSelection([&](const fs::path& path){
            m_projectSavePath = path;
            saveProject();
        })
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
    m_editedScene = nullptr;
    m_selectedEntity = Entity();
    m_editorCamera = EditorCamera();

    editScene(m_project.startScene());
    reloadScriptLib();
    m_imguiSettingsNeedReload = true;
}

void Editor::processDroppedFiles()
{
    fs::path path;
    while (m_window->popDroppedFile(path))
    {
        if (json::accept(std::ifstream(path)))
        {
            json jsn = json::parse(std::ifstream(path));
            if (jsn.find("scenes") != jsn.end() && jsn.find("imguiSettings") != jsn.end())
            {
                openProject(path);
                continue;
            }
        }

        if (stbi_info(path.c_str(), nullptr, nullptr, nullptr) == 1)
            continue;

        if (m_editedScene != nullptr)
        {
            m_editedScene->assetManager().registerMesh(path);
            continue;
        }
    }
}

}