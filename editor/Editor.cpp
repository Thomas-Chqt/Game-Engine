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
#include "UtilsCPP/UniquePtr.hpp"
#include "imguiPanels/SceneGraphPanel.hpp"
#include "imguiPanels/EntityInspectorPanel.hpp"
#include "imguiPanels/ViewportPanel.hpp"
#include "TFD/tinyfiledialogs.h"

namespace GE
{

Editor::Editor()
{
    resetEditorInputs();
}

void Editor::onUpdate()
{
    if (m_viewportFBuffSizeIsDirty)
    {
        updateVPFrameBuff();
        m_viewportFBuffSizeIsDirty = false;
    }
        
    m_renderer.beginScene(m_editorCamera.getRendererCam(), m_viewportFBuff.staticCast<gfx::RenderTarget>());
    {
        if (m_editedScene)
            m_editedScene->submitForRendering(m_renderer);
    }
    m_renderer.endScene();

    m_editorInputContext.dispatchInputs();
}

void Editor::onImGuiRender()
{
    ImGui::DockSpaceOverViewport();

    static bool showDemoWindow = false;
    static bool showProjectProperties = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open", "Ctrl+O"))
            {
                if (char* path = tinyfd_openFileDialog("", "", 0, nullptr, nullptr, 0))
                    m_project = Project(path);
            }

            if (ImGui::MenuItem("Save", "Ctrl+S"))
            {
                if (m_project.path().isEmpty())
                {
                    if (char* path = tinyfd_saveFileDialog("", m_project.name() + ".json", 0, nullptr, nullptr))
                    {
                        m_project.setPath(path);
                        m_project.saveProject();
                    }
                }
                else 
                    m_project.saveProject();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Project"))
        {
            if (ImGui::MenuItem("Properties"))
                showProjectProperties = true;

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug"))
        {
            
            if (ImGui::MenuItem("Demo window"))
                showDemoWindow = true;
            ImGui::EndMenu();
        }
            
        ImGui::EndMainMenuBar();
    }

    if (showProjectProperties)
        ImGui::OpenPopup("Project properties");
    if (ImGui::BeginPopupModal("Project properties", &showProjectProperties))
    {
        ImGui::Text("%s", (const char*)m_project.name());
        ImGui::EndPopup();
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

    if (showDemoWindow)
        ImGui::ShowDemoWindow(&showDemoWindow);
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

void Editor::editScene(Scene* scene)
{
    if (m_editedScene)
        m_editedScene->unload();
    scene->load(m_renderer.graphicAPI());
    m_editedScene = scene;

    m_editorCamera = EditorCamera();
    m_selectedEntity = Entity();
}

}