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
#include "Game.hpp"
#include "Graphics/Event.hpp"
#include "InputManager/RawInput.hpp"
#include "InputManager/Mapper.hpp"
#include "Math/Constants.hpp"
#include "Project.hpp"
#include "Scene.hpp"
#include "UtilsCPP/UniquePtr.hpp"
#include <utility>
#include "ViewportFrameBuffer.hpp"
#include "imguiPanels/SceneGraphPanel.hpp"
#include "imguiPanels/EntityInspectorPanel.hpp"
#include "imguiPanels/ViewportPanel.hpp"

namespace GE
{

Editor::Editor()
{
    Scene defaultScene;
    defaultScene.assetManager().registerMesh(RESSOURCES_DIR"/cube.glb");
    defaultScene.assetManager().registerMesh(RESSOURCES_DIR"/chess_set/chess_set.gltf");

    GE::Entity player = defaultScene.newEntity("player");
    player.emplace<GE::CameraComponent>((float)(60 * (PI / 180.0F)), 10000.0f, 0.01f);
    player.emplace<GE::LightComponent>(GE::LightComponent::Type::point, WHITE3, 1.0f);

    m_project = Project("/Users/thomas/Library/Mobile Documents/com~apple~CloudDocs/Visual Studio Code/C++/Projects/Game-Engine/editor");
    m_project.setRessourceDir("ressources");
    
    m_project.game().addScene("default_scene", std::move(defaultScene));
    m_project.game().setStartScene("default_scene");
    
    resetEditorInputs();

    editScene(&m_project.game().getScene("default_scene"));
}

void Editor::onUpdate()
{
    m_viewportFBuff.update(*m_window, m_renderer.graphicAPI());

    m_renderer.beginScene(m_editorCamera.getRendererCam(), m_viewportFBuff.getAsRenderTarget());
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
    
    ViewportPanel(m_viewportFBuff)
        .onResize(utils::Func<void (utils::uint32, utils::uint32)>(m_viewportFBuff, &ViewportFrameBuffer::resize))
        .render();

    SceneGraphPanel(m_editedScene, m_selectedEntity)
        .onEntitySelect([&](Entity entity){ m_selectedEntity = entity; })
        .render();

    EntityInspectorPanel(m_project, m_editedScene, m_selectedEntity)
        .render();
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