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
#include "Scene.hpp"
#include "UtilsCPP/String.hpp"
#include "UtilsCPP/UniquePtr.hpp"
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
    ActionInput& quitEditorIpt = m_editorInputContext.newInput<ActionInput>("quit_editor");
    quitEditorIpt.callback = utils::Func<void()>(*(Application*)this, &Application::terminate);
    Range2DInput& editorCamMoveIpt = m_editorInputContext.newInput<Range2DInput>("editor_cam_move");
    editorCamMoveIpt.callback = utils::Func<void(math::vec2f)>(m_editorCamera, &EditorCamera::move);
    Range2DInput& editorCamRotateIpt = m_editorInputContext.newInput<Range2DInput>("editor_cam_rotate");
    editorCamRotateIpt.callback = utils::Func<void(math::vec2f)>(m_editorCamera, &EditorCamera::rotate);

    ImGui::GetIO().IniFilename = nullptr;

    m_projectName = "new_project";
    m_imguiSettings = DEFAULT_IMGUI_INI;
    m_imguiSettingsNeedReload = true;

    auto& defautScene = *m_scenes.insert(Scene("default_scene"));
    defautScene.newEntity("cube");

    editScene(&defautScene);

    resetEditorInputs();
}

void Editor::onUpdate()
{
    if (m_imguiSettingsNeedReload)
    {
        ImGui::LoadIniSettingsFromMemory(m_imguiSettings);
        m_imguiSettingsNeedReload = false;
    }

    updateVPFrameBuff();

    m_renderer.beginScene(m_editorCamera.getRendererCam(), m_viewportFBuff.staticCast<gfx::RenderTarget>());
    {
        if (m_editedScene)
            m_editedScene->submitForRendering(m_renderer);
    }
    m_renderer.endScene();

    if (ImGui::GetIO().WantSaveIniSettings)
    {
        m_imguiSettings = ImGui::SaveIniSettingsToMemory();
        ImGui::GetIO().WantSaveIniSettings = false;
    }

    m_editorInputContext.dispatchInputs();
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

void Editor::openProject(const utils::String& filePath)
{
    m_projectFilePath = filePath;

    std::ifstream f(m_projectFilePath);
    json jsn = json::parse(f);

    auto nameIt = jsn.find("name");
    m_projectName = nameIt != jsn.end() ? utils::String(nameIt->template get<std::string>().c_str()) : "";

    auto ressDirIt = jsn.find("ressourcesDir");
    m_projectRessourcesDir = ressDirIt != jsn.end() ? utils::String(ressDirIt->template get<std::string>().c_str()) : "";

    auto imguiSettIt = jsn.find("imguiSettings");
    if (imguiSettIt != jsn.end())
    {
        m_imguiSettings = utils::String(imguiSettIt->template get<std::string>().c_str());
        m_imguiSettingsNeedReload = true;
    }

    m_scenes.clear();
    auto scenesIt = jsn.find("scenes");
    if (scenesIt != jsn.end())
    {
        for (auto& scene : *scenesIt)
            m_scenes.insert(scene.template get<Scene>());
    }

    auto startSceneNameIt = jsn.find("startScene");
    if (startSceneNameIt != jsn.end())  
    {
        auto startSceneIt = m_scenes.find(utils::String(startSceneNameIt->template get<std::string>().c_str()));
        m_startScene = startSceneIt != m_scenes.end() ? &*startSceneIt : nullptr;
    }
    else
        m_startScene = nullptr;

    m_editedScene = nullptr;
    m_selectedEntity = Entity();
    m_editorCamera = EditorCamera();
}

void Editor::saveProject()
{
    json jsn;

    jsn["name"] = std::string(m_projectName);
    jsn["ressourcesDir"] = std::string(m_projectRessourcesDir);
    jsn["imguiSettings"] = std::string(m_imguiSettings);

    json scenesJsn = json::array();
    for (auto& scene : m_scenes)
        scenesJsn.emplace_back(scene);
    jsn["scenes"] = scenesJsn;

    jsn["startScene"] = m_startScene ? m_startScene->name() : "";
    
    std::ofstream(m_projectFilePath) << json(jsn).dump(4);
}

void Editor::editScene(Scene* scene)
{
    if (m_editedScene)
        m_editedScene->unload();
    scene->load(m_renderer.graphicAPI());

    m_editedScene = scene;
    m_selectedEntity = Entity();
    m_editorCamera = EditorCamera();
}

void Editor::resetEditorInputs()
{
    Mapper<KeyboardButton, Range2DInput>::Descriptor inputMapperDesc;

    ActionInput& quitEditorIpt = m_editorInputContext.getInput<ActionInput>("quit_editor");
    auto quitEditorIptMapper = utils::makeUnique<Mapper<KeyboardButton, ActionInput>>(KeyboardButton::esc, quitEditorIpt);
    quitEditorIpt.mappers[0] = quitEditorIptMapper.staticCast<IMapper>();
    quitEditorIpt.mappers[1].clear();

    Range2DInput& editorCamMoveIpt = m_editorInputContext.getInput<Range2DInput>("editor_cam_move");
    inputMapperDesc.xPos = KeyboardButton::d;
    inputMapperDesc.xNeg = KeyboardButton::a;
    inputMapperDesc.yPos = KeyboardButton::w;
    inputMapperDesc.yNeg = KeyboardButton::s;
    auto editorCamMoveIptMapper = utils::makeUnique<Mapper<KeyboardButton, Range2DInput>>(inputMapperDesc, editorCamMoveIpt);
    editorCamMoveIpt.mappers[0] = editorCamMoveIptMapper.staticCast<IMapper>();
    editorCamMoveIpt.mappers[1].clear();

    Range2DInput& editorCamRotateIpt = m_editorInputContext.getInput<Range2DInput>("editor_cam_rotate");
    inputMapperDesc.xPos = KeyboardButton::down;
    inputMapperDesc.xNeg = KeyboardButton::up;
    inputMapperDesc.yPos = KeyboardButton::right;
    inputMapperDesc.yNeg = KeyboardButton::left;
    auto editorCamRotateIptMapper = utils::makeUnique<Mapper<KeyboardButton, Range2DInput>>(inputMapperDesc, editorCamRotateIpt);
    editorCamRotateIpt.mappers[0] = editorCamRotateIptMapper.staticCast<IMapper>();
    editorCamRotateIpt.mappers[1].clear();
}

}