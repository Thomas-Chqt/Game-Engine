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
#include "Game/StarterContent.hpp"
#include "Graphics/Event.hpp"
#include "Graphics/KeyCodes.hpp"
#include "Graphics/Platform.hpp"
#include "Graphics/RenderTarget.hpp"
#include "Math/Constants.hpp"
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
            m_game->onUpdate();
            scriptSystem();
        }
        else
            updateEditorCamera();

        updateVPFrameBuff();

        Renderer::Camera camera = m_gameRunning ? getActiveCameraSystem() : getEditorCamera();
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
}

void Engine::onEvent(gfx::Event& event)
{
    event.dispatch<gfx::KeyDownEvent>([&](gfx::KeyDownEvent& event) {
        if (event.isRepeat() == false)
            m_pressedKeys.insert(event.keyCode());                     
    });
    
    event.dispatch<gfx::KeyUpEvent>([&](gfx::KeyUpEvent& event) {
        m_pressedKeys.remove(m_pressedKeys.find(event.keyCode()));
    });

    if (m_gameRunning)
        m_game->onEvent(event);
}

void Engine::onImGuiRender()
{
    ImGui::DockSpaceOverViewport();
    
    drawViewportPanel();
    drawSceneGraphPanel();
    drawEntityInspectorPanel();
    drawFPSPanel();

    if (m_gameRunning)
        m_game->onImGuiRender();
}

void Engine::updateEditorCamera()
{
    math::vec3f dir = { 0.0, 0.0, 0.0 };
    for (const auto& key : m_pressedKeys)
    {
        switch (key)
        {
            case W_KEY: dir += math::vec3f{ 0, 0,  1}; break;
            case A_KEY: dir += math::vec3f{-1, 0,  0}; break;
            case S_KEY: dir += math::vec3f{ 0, 0, -1}; break;
            case D_KEY: dir += math::vec3f{ 1, 0,  0}; break;

            case UP_KEY:    m_editorCameraRot.x -= 0.05; break;
            case LEFT_KEY:  m_editorCameraRot.y -= 0.05; break;
            case DOWN_KEY:  m_editorCameraRot.x += 0.05; break;
            case RIGHT_KEY: m_editorCameraRot.y += 0.05; break;
        }
    }
    m_editorCameraPos += math::mat3x3::rotation(m_editorCameraRot) * dir.normalized() * 0.2;
}

Renderer::Camera Engine::getEditorCamera()
{
    Renderer::Camera cam;

    float zs = 10000.0F / (10000.0F - 0.01F);
    float ys = 1.0F / std::tan((float)(60 * (PI / 180.0F)) * 0.5F);
    float xs = ys; // (ys / aspectRatio)

    cam.projectionMatrix = math::mat4x4(xs,  0,  0,           0,
                                         0, ys,  0,           0,
                                         0,  0, zs, -0.01F * zs,
                                         0,  0,  1,           0);

    cam.viewMatrix = (math::mat4x4::translation(m_editorCameraPos) * math::mat4x4::rotation(m_editorCameraRot)).inversed();
    return cam;
}

void Engine::updateVPFrameBuff()
{
    float xScale, yScale;
    m_window->getFrameBufferScaleFactor(&xScale, &yScale);

    math::vec2f viewportPanelfBuffSize = math::vec2f(
        (m_viewportPanelSize.x == 0 ? 1 : m_viewportPanelSize.x) * xScale,
        (m_viewportPanelSize.y == 0 ? 1 : m_viewportPanelSize.y) * yScale
    );
    
    if (m_viewportFBuff)
    {
        utils::SharedPtr<gfx::Texture> fbColorTex = m_viewportFBuff->colorTexture();
        if (fbColorTex->height() == viewportPanelfBuffSize.y && fbColorTex->width() == viewportPanelfBuffSize.x)
            return;
    }

    gfx::Texture::Descriptor colorTextureDescriptor;
    colorTextureDescriptor.width = viewportPanelfBuffSize.x;
    colorTextureDescriptor.height = viewportPanelfBuffSize.y;
    colorTextureDescriptor.pixelFormat = gfx::PixelFormat::BGRA;
    colorTextureDescriptor.usage = gfx::Texture::Usage::ShaderReadAndRenderTarget;
    
    gfx::Texture::Descriptor depthTextureDescriptor = gfx::Texture::Descriptor::depthTextureDescriptor(viewportPanelfBuffSize.x, viewportPanelfBuffSize.y);

    gfx::FrameBuffer::Descriptor fBuffDesc;
    fBuffDesc.colorTexture = m_renderer.graphicAPI().newTexture(colorTextureDescriptor);
    fBuffDesc.depthTexture = m_renderer.graphicAPI().newTexture(depthTextureDescriptor);
    m_viewportFBuff = m_renderer.graphicAPI().newFrameBuffer(fBuffDesc);
}

}