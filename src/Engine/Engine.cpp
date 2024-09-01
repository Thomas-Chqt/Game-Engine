/*
 * ---------------------------------------------------
 * Engine.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/31 12:53:04
 * ---------------------------------------------------
 */

#include "Engine/Engine.hpp"
#include "Game-Engine/Game.hpp"
#include "Game-Engine/Input.hpp"
#include "Game-Engine/InputContext.hpp"
#include "Game-Engine/Mapper.hpp"
#include "Game-Engine/RawInput.hpp"
#include "Game/StarterContent.hpp"
#include "Graphics/Event.hpp"
#include "Graphics/Platform.hpp"
#include "Graphics/RenderTarget.hpp"
#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/UniquePtr.hpp"
#include <cmath>

namespace GE
{

void openProject(const utils::String& filepath)
{
}

void Engine::run()
{
    m_editorRunning = true;
    while (m_editorRunning)
    {
        gfx::Platform::shared().pollEvents();
        if (m_gameRunning)
        {
            m_game->inputContext().dispatchInputs();
            m_game->onUpdate();
            scriptSystem();
        }
        else
            m_editorInputContext.dispatchInputs();

        if (m_viewportPanelSizeIsDirty)
        {
            updateVPFrameBuff();
            m_viewportPanelSizeIsDirty = false;
        }        

        Renderer::Camera camera = m_gameRunning ? getActiveCameraSystem() : m_editorCamera.getRendererCam();
        m_renderer.beginScene(camera, m_viewportFBuff.staticCast<gfx::RenderTarget>());
        {
            addLightsSystem();
            addRenderableSystem();
        }
        m_renderer.endScene();

        m_renderer.render();
    }
}

Engine::~Engine()
{
    m_renderer.setOnImGuiRender(utils::Func<void ()>());
    gfx::Platform::shared().clearCallbacks(this);
}

Engine::Engine()
    : m_window(gfx::Platform::shared().newWindow(1280, 720)),
      m_renderer(gfx::Platform::shared().newGraphicAPI(m_window))
{
    m_game = utils::makeUnique<StarterContent>().staticCast<Game>();

    gfx::Platform::shared().addEventCallBack(utils::Func<void(gfx::Event&)>(*this, &Engine::onEvent));
    m_renderer.setOnImGuiRender(utils::Func<void()>(*this, &Engine::onImGuiRender));

    ActionInput& quitEditorIpt = m_editorInputContext.newInput<ActionInput>("quit_editor");
    quitEditorIpt.callback = [&](){ m_editorRunning = false; };
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

void Engine::onEvent(gfx::Event& event)
{
    if (m_gameRunning)
    {
        if (event.dispatch(utils::Func<void(gfx::InputEvent&)>(m_game->inputContext(), &InputContext::onInputEvent)))
            return;
        if (event.dispatch(utils::Func<void(gfx::WindowResizeEvent&)>(*m_game, &Game::onWindowResizeEvent)))
            return;
        if (event.dispatch(utils::Func<void(gfx::WindowRequestCloseEvent&)>(*m_game, &Game::onWindowRequestCloseEvent)))
            return;
    }
    else
    {
        if (event.dispatch(utils::Func<void(gfx::InputEvent&)>(m_editorInputContext, &InputContext::onInputEvent)))
            return;
        if (event.dispatch<gfx::WindowResizeEvent>([&](gfx::WindowResizeEvent& windowResizeEvent){
            //
        })) return;
        if (event.dispatch<gfx::WindowRequestCloseEvent>([&](gfx::WindowRequestCloseEvent& windowRequestCloseEvent){
            m_editorRunning = false;
        })) return;
    }
}

void Engine::onImGuiRender()
{
    ImGui::DockSpaceOverViewport();
    
    drawViewportPanel();
    drawSceneGraphPanel();
    drawEntityInspectorPanel();
    drawFPSPanel();
}

void Engine::updateVPFrameBuff()
{
    float xScale, yScale;
    m_window->getFrameBufferScaleFactor(&xScale, &yScale);

    utils::uint32 newFrameBufferWidth = (utils::uint32)((float)m_viewportPanelSize.x * xScale);
    utils::uint32 newFrameBufferHeight = (utils::uint32)((float)m_viewportPanelSize.y * yScale);
    
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

}