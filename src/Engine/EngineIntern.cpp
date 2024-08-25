/*
 * ---------------------------------------------------
 * EngineIntern.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/06/20 17:15:45
 * ---------------------------------------------------
 */

#include "Engine/EngineIntern.hpp"
#include "ECS/ECSView.hpp"
#include "ECS/InternalComponents.hpp"
#include "Engine/EditorCamera.hpp"
#include "Game-Engine/AssetManager.hpp"
#include "Game-Engine/Engine.hpp"
#include "Game-Engine/Game.hpp"
#include "Game-Engine/Input.hpp"
#include "Game-Engine/InputContext.hpp"
#include "Game-Engine/Mapper.hpp"
#include "Game-Engine/RawInput.hpp"
#include "Graphics/Event.hpp"
#include "Graphics/FrameBuffer.hpp"
#include "Graphics/GraphicAPI.hpp"
#include "Graphics/Platform.hpp"
#include "GPURessourceManager.hpp"
#include "Graphics/Texture.hpp"
#include "Math/Vector.hpp"
#include "Renderer/Renderer.hpp"
#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/Types.hpp"
#include "UtilsCPP/UniquePtr.hpp"
#include <cassert>
#include <cmath>
#include <cstring>
#include <utility>

namespace GE
{

void Engine::init()
{
    EngineIntern::init();
}

void EngineIntern::runGame(utils::UniquePtr<Game>&& game)
{
    m_game = std::move(game);

    m_game->onSetup();
    ECSView<ScriptComponent>(m_game->activeScene()).onEach([](Entity, ScriptComponent& scriptComponent) {
        scriptComponent.instance->onSetup();
    });

    m_gameRunning = true;
    while (m_gameRunning)
    {
        gfx::Platform::shared().pollEvents();
        m_game->inputContext().dispatchInputs();

        m_game->onUpdate();
        scriptSystem();

        Renderer& renderer = Renderer::shared();
        renderer.beginScene(getActiveCameraSystem(), *m_mainWindow);
        {
            addLightsSystem();
            addRenderableSystem();
        }
        renderer.endScene();

        renderer.render();
    }
}

void EngineIntern::editorForGame(utils::UniquePtr<Game>&& game)
{
    m_game = std::move(game);

    Renderer::shared().setOnImGuiRender(utils::Func<void()>(*this, &EngineIntern::onImGuiRender));

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

        Renderer& renderer = Renderer::shared();
        Renderer::Camera camera = m_gameRunning ? getActiveCameraSystem() : m_editorCamera.getRendererCam();
        renderer.beginScene(camera, m_viewportFBuff);
        {
            addLightsSystem();
            addRenderableSystem();
        }
        renderer.endScene();

        renderer.render();
    }
}

EngineIntern::~EngineIntern()
{
    AssetManager::terminate();

    Renderer::terminate();

    GPURessourceManager::terminate();

    gfx::Platform::shared().clearCallbacks(this);
    gfx::Platform::terminate();
}

EngineIntern::EngineIntern()
{
    gfx::Platform::init();
    gfx::Platform::shared().addEventCallBack(utils::Func<void(gfx::Event&)>(*this, &EngineIntern::onEvent), this);

    m_mainWindow = gfx::Platform::shared().newWindow(1280, 720);
    utils::SharedPtr<gfx::GraphicAPI> graphicAPI = gfx::Platform::shared().newGraphicAPI(m_mainWindow);
    graphicAPI->initImgui();
    GPURessourceManager::init(graphicAPI);

    Renderer::init();

    AssetManager::init();

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

void EngineIntern::onEvent(gfx::Event& event)
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

void EngineIntern::onImGuiRender()
{
    ImGui::DockSpaceOverViewport();
    
    drawViewportPanel();
    drawSceneGraphPanel();
    drawEntityInspectorPanel();
    drawFPSPanel();
}

void EngineIntern::updateVPFrameBuff()
{
    float xScale, yScale;
    m_mainWindow->getFrameBufferScaleFactor(&xScale, &yScale);

    utils::uint32 newFrameBufferWidth = (utils::uint32)((float)m_viewportPanelSize.x * xScale);
    utils::uint32 newFrameBufferHeight = (utils::uint32)((float)m_viewportPanelSize.y * yScale);
    
    gfx::Texture::Descriptor colorTextureDescriptor;
    colorTextureDescriptor.width = newFrameBufferWidth;
    colorTextureDescriptor.height = newFrameBufferHeight;
    colorTextureDescriptor.pixelFormat = gfx::PixelFormat::BGRA;
    colorTextureDescriptor.usage = gfx::Texture::Usage::ShaderReadAndRenderTarget;
    
    gfx::Texture::Descriptor depthTextureDescriptor = gfx::Texture::Descriptor::depthTextureDescriptor(newFrameBufferWidth, newFrameBufferHeight);

    gfx::FrameBuffer::Descriptor fBuffDesc;
    fBuffDesc.colorTexture = GPURessourceManager::shared().newTexture(colorTextureDescriptor);
    fBuffDesc.depthTexture = GPURessourceManager::shared().newTexture(depthTextureDescriptor);
    m_viewportFBuff = GPURessourceManager::shared().newFrameBuffer(fBuffDesc);
}

}
