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
#include "UtilsCPP/String.hpp"
#include "UtilsCPP/UniquePtr.hpp"
#include "imgui/NewProjectPopupModal.hpp"
#include "imgui/ProjectPropertiesPopupModal.hpp"
#include "imgui/SceneGraphPanel.hpp"
#include "imgui/EntityInspectorPanel.hpp"
#include "imgui/ViewportPanel.hpp"
#include "TFD/tinyfiledialogs.h"
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#define DEFAULT_IMGUI_INI\
    "[Window][WindowOverViewport_11111111]\n"\
    "Pos=0,19\n"\
    "Size=1280,701\n"\
    "Collapsed=0\n"\
    "\n"\
    "[Window][Debug##Default]\n"\
    "Pos=60,60\n"\
    "Size=400,400\n"\
    "Collapsed=0\n"\
    "\n"\
    "[Window][Entity inspector]\n"\
    "Pos=999,494\n"\
    "Size=281,226\n"\
    "Collapsed=0\n"\
    "DockId=0x00000004,0\n"\
    "\n"\
    "[Window][Scene graph]\n"\
    "Pos=999,19\n"\
    "Size=281,473\n"\
    "Collapsed=0\n"\
    "DockId=0x00000003,0\n"\
    "\n"\
    "[Window][viewport]\n"\
    "Pos=0,19\n"\
    "Size=997,701\n"\
    "Collapsed=0\n"\
    "DockId=0x00000001,0\n"\
    "\n"\
    "[Docking][Data]\n"\
    "DockSpace ID=0x7C6B3D9B Window=0xA87D555D Pos=942,394 Size=1280,701 Split=X Selected=0x0BA3B4F3\n"\
    "DockNode  ID=0x00000001 Parent=0x7C6B3D9B SizeRef=1185,701 CentralNode=1 Selected=0x0BA3B4F3\n"\
    "DockNode  ID=0x00000002 Parent=0x7C6B3D9B SizeRef=281,701 Split=Y Selected=0xF5BE1C77\n"\
    "DockNode  ID=0x00000003 Parent=0x00000002 SizeRef=168,473 Selected=0xF5BE1C77\n"\
    "DockNode  ID=0x00000004 Parent=0x00000002 SizeRef=168,226 Selected=0xD3D12213\n"


namespace GE
{

Editor::Editor()
{
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::LoadIniSettingsFromMemory(DEFAULT_IMGUI_INI);
    resetEditorInputs();
}

void Editor::onUpdate()
{
    if (m_viewportFBuffSizeIsDirty)
    {
        updateVPFrameBuff();
        m_viewportFBuffSizeIsDirty = false;
    }

    if (m_uiStates.imguiSettingsNeedReload)
    {
        ImGui::LoadIniSettingsFromMemory(m_project.imguiSettings);
        m_uiStates.imguiSettingsNeedReload = false;
    }

    m_renderer.beginScene(m_editorCamera.getRendererCam(), m_viewportFBuff.staticCast<gfx::RenderTarget>());
    {
        if (m_editedScene)
            m_editedScene->submitForRendering(m_renderer);
    }
    m_renderer.endScene();

    if (ImGui::GetIO().WantSaveIniSettings)
    {
        m_project.imguiSettings = ImGui::SaveIniSettingsToMemory();
        ImGui::GetIO().WantSaveIniSettings = false;
    }

    m_editorInputContext.dispatchInputs();
}

void Editor::onImGuiRender()
{
    ImGui::DockSpaceOverViewport();

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New project"))
            {
                m_uiStates.newProjectName = "new_project";
                m_uiStates.newProjectPath = "";
                m_uiStates.showNewProjectPopupModal = true;
            }

            if (ImGui::MenuItem("Open"))
            {
                if (char* path = tinyfd_openFileDialog("Open project", "", 0, nullptr, nullptr, 0))
                    openProject(path);
            }

            if (ImGui::MenuItem("Save", nullptr, false, m_openProjFilePath.isEmpty() == false))
                saveProject();

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Project"))
        {
            if (ImGui::MenuItem("Properties", nullptr, false, m_openProjFilePath.isEmpty() == false))
                m_uiStates.showProjectProperties = true;

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug"))
        {
            if (ImGui::MenuItem("Show demo window"))
                m_uiStates.showDemoWindow = true;

            ImGui::EndMenu();
        }
            
        ImGui::EndMainMenuBar();
    }

    ViewportPanel(m_viewportFBuff->colorTexture())
        .onResize([&](utils::uint32 w , utils::uint32 h){
            m_viewportFBuffW = w;
            m_viewportFBuffH = h;
            m_viewportFBuffSizeIsDirty = true;
        })
        .render();

    SceneGraphPanel(m_editedScene, m_selectedEntity)
        .onEntitySelect([&](Entity entity){ m_selectedEntity = entity; })
        .render();

    EntityInspectorPanel(m_project, m_editedScene, m_selectedEntity)
        .render();

    NewProjectPopupModal(m_uiStates.showNewProjectPopupModal, m_uiStates.newProjectName, m_uiStates.newProjectPath)
        .onCreatePressed([&](){ 
            newProject(m_uiStates.newProjectName, m_uiStates.newProjectPath);
            m_uiStates.showNewProjectPopupModal = false;
        })
        .render();

    ProjectPropertiesPopupModal(m_uiStates.showProjectProperties, m_project)
        .onClose([&](){m_uiStates.showProjectProperties = false; })
        .render();

    if (m_uiStates.showDemoWindow)
        ImGui::ShowDemoWindow(&m_uiStates.showDemoWindow);
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

void Editor::newProject(const utils::String& name, const utils::String& path)
{
    m_openProjFilePath = path + "/" + name + ".geproj";
    m_openProjBaseDir = path;

    m_project = Project();
    m_project.name = name;
    m_project.imguiSettings = ImGui::SaveIniSettingsToMemory();

    saveProject();
}

void Editor::openProject(const utils::String& filePath)
{
    m_openProjFilePath = filePath;
    m_openProjBaseDir = filePath.substr(0, filePath.lastIndexOf('/'));

    std::ifstream f(m_openProjFilePath);
    m_project = json::parse(f);

    m_uiStates.imguiSettingsNeedReload = true;
}

void Editor::saveProject()
{
    std::ofstream f(m_openProjFilePath);
    f << json(m_project).dump(4);
}

void Editor::editScene(Scene* scene)
{
    if (m_editedScene)
        m_editedScene->unload();
    scene->load(m_renderer.graphicAPI());
    m_editedScene = scene;

    m_editorCamera = EditorCamera();
    m_selectedEntity = Entity();
}

void Editor::updateVPFrameBuff()
{
    float xScale, yScale;
    m_window->getFrameBufferScaleFactor(&xScale, &yScale);

    utils::uint32 newFrameBufferWidth = (utils::uint32)((float)m_viewportFBuffW * xScale);
    utils::uint32 newFrameBufferHeight = (utils::uint32)((float)m_viewportFBuffH * yScale);
    
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

void Editor::resetEditorInputs()
{
    m_editorInputContext.clear();

    ActionInput& quitEditorIpt = m_editorInputContext.newInput<ActionInput>("quit_editor");
    quitEditorIpt.callback = utils::Func<void()>(*(Application*)this, &Application::terminate);
    auto quitEditorIptMapper = utils::makeUnique<Mapper<KeyboardButton, ActionInput>>(KeyboardButton::esc, quitEditorIpt);
    quitEditorIpt.mappers[0] = quitEditorIptMapper.staticCast<IMapper>();

    Range2DInput& editorCamRotateIpt = m_editorInputContext.newInput<Range2DInput>("editor_cam_rotate");
    editorCamRotateIpt.callback = utils::Func<void(math::vec2f)>(m_editorCamera, &EditorCamera::rotate);
    Mapper<KeyboardButton, Range2DInput>::Descriptor inputMapperDesc;
    inputMapperDesc.xPos = KeyboardButton::down;
    inputMapperDesc.xNeg = KeyboardButton::up;
    inputMapperDesc.yPos = KeyboardButton::right;
    inputMapperDesc.yNeg = KeyboardButton::left;
    auto editorCamRotateIptMapper = utils::makeUnique<Mapper<KeyboardButton, Range2DInput>>(inputMapperDesc, editorCamRotateIpt);
    editorCamRotateIpt.mappers[0] = editorCamRotateIptMapper.staticCast<IMapper>();
    
    Range2DInput& editorCamMoveIpt = m_editorInputContext.newInput<Range2DInput>("editor_cam_move");
    editorCamMoveIpt.callback = utils::Func<void(math::vec2f)>(m_editorCamera, &EditorCamera::move);
    inputMapperDesc.xPos = KeyboardButton::d;
    inputMapperDesc.xNeg = KeyboardButton::a;
    inputMapperDesc.yPos = KeyboardButton::w;
    inputMapperDesc.yNeg = KeyboardButton::s;
    auto editorCamMoveIptMapper = utils::makeUnique<Mapper<KeyboardButton, Range2DInput>>(inputMapperDesc, editorCamMoveIpt);
    editorCamMoveIpt.mappers[0] = editorCamMoveIptMapper.staticCast<IMapper>();
}

}