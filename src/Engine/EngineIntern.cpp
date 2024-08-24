/*
 * ---------------------------------------------------
 * EngineIntern.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/06/20 17:15:45
 * ---------------------------------------------------
 */

#include "Engine/EngineIntern.hpp"
#include "Game-Engine/AssetManager.hpp"
#include "Game-Engine/Engine.hpp"
#include "Game-Engine/Game.hpp"
#include "Graphics/Event.hpp"
#include "Graphics/FrameBuffer.hpp"
#include "Graphics/GraphicAPI.hpp"
#include "Graphics/Platform.hpp"
#include "GPURessourceManager.hpp"
#include "Graphics/Texture.hpp"
#include "InputManager/InputManagerIntern.hpp"
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
    InputManagerIntern::shared().disableEditorInputs();
    InputManagerIntern::shared().enableGameInputs();

    m_gameRunning = true;
    while (m_gameRunning)
    {
        gfx::Platform::shared().pollEvents();
        InputManagerIntern::shared().dispatchInputs();

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

    m_editorCamera.setupFreeCam();
    InputManagerIntern::shared().disableGameInputs();
    InputManagerIntern::shared().enableEditorInputs();

    m_editorRunning = true;
    while (m_editorRunning)
    {
        gfx::Platform::shared().pollEvents();
        InputManagerIntern::shared().dispatchInputs();

        if (m_gameRunning)
        {   
            m_game->onUpdate();
            scriptSystem();
        }

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
    InputManagerIntern::terminate();
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
    Renderer::shared().setOnImGuiRender(utils::Func<void()>(*this, &EngineIntern::onImGuiRender));

    AssetManager::init();
    InputManagerIntern::init();
}

void EngineIntern::onEvent(gfx::Event& event)
{
    if (m_editorRunning)
    {
        if (event.dispatch<gfx::WindowResizeEvent>([&](gfx::WindowResizeEvent& windowResizeEvent){
            //
        })) return;
        if (event.dispatch<gfx::WindowRequestCloseEvent>([&](gfx::WindowRequestCloseEvent& windowRequestCloseEvent){
            m_editorRunning = false;
        })) return;
    }
    else
    {
        if (event.dispatch<gfx::WindowResizeEvent>([&](gfx::WindowResizeEvent& windowResizeEvent){
            m_game->onWindowResizeEvent(windowResizeEvent);
        })) return;
        if (event.dispatch<gfx::WindowRequestCloseEvent>([&](gfx::WindowRequestCloseEvent& windowRequestCloseEvent){
            m_game->onWindowRequestCloseEvent(windowRequestCloseEvent);
        })) return;
    }
}

void EngineIntern::onImGuiRender()
{
    if (m_editorRunning)
    {
        ImGui::DockSpaceOverViewport();
        
        drawViewportPanel();
        drawSceneGraphPanel();
        drawEntityInspectorPanel();
        drawFPSPanel();
    }

    if (m_gameRunning)
        m_game->onImGuiRender();
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
