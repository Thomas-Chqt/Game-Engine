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
#include "Graphics/KeyCodes.hpp"
#include "Graphics/Platform.hpp"
#include "GPURessourceManager.hpp"
#include "Graphics/Texture.hpp"
#include "Math/Constants.hpp"
#include "Renderer/Renderer.hpp"
#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/UniquePtr.hpp"
#include <cassert>
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

    m_gameRunning = true;
    while (m_gameRunning)
    {
        gfx::Platform::shared().pollEvents();

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

    gfx::FrameBuffer::Descriptor fBuffDesc;
    fBuffDesc.colorTexture = GPURessourceManager::shared().newTexture(gfx::Texture::Descriptor::texture2dDescriptor(800, 600));
    fBuffDesc.depthTexture = GPURessourceManager::shared().newTexture(gfx::Texture::Descriptor::depthTextureDescriptor(800, 600));
    m_viewportFBuff = GPURessourceManager::shared().newFrameBuffer(fBuffDesc);

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

        Renderer& renderer = Renderer::shared();
        Renderer::Camera camera = m_gameRunning ? getActiveCameraSystem() : getEditorCamera();
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

    m_mainWindow = gfx::Platform::shared().newWindow(800, 600);
    utils::SharedPtr<gfx::GraphicAPI> graphicAPI = gfx::Platform::shared().newGraphicAPI(m_mainWindow);
    graphicAPI->initImgui();
    GPURessourceManager::init(graphicAPI);

    Renderer::init();
    Renderer::shared().setOnImGuiRender(utils::Func<void()>(*this, &EngineIntern::onImGuiRender));

    AssetManager::init();
}

void EngineIntern::onEvent(gfx::Event& event)
{
    event.dispatch<gfx::KeyDownEvent>([&](gfx::KeyDownEvent& event) {
        if (event.isRepeat() == false)
            m_pressedKeys.insert(event.keyCode());
    });
    
    event.dispatch<gfx::KeyUpEvent>([&](gfx::KeyUpEvent& event) {
        m_pressedKeys.remove(m_pressedKeys.find(event.keyCode()));
    });

    assert(m_game);
    m_game->onEvent(event);
}

void EngineIntern::updateEditorCamera()
{
    math::vec3f dir = { 0.0, 0.0, 0.0 };
    for (const auto& key : GE::Engine::shared().pressedKeys())
    {
        switch (key)
        {
            case W_KEY:     dir += math::vec3f{ 0, 0,  1};         break;
            case A_KEY:     dir += math::vec3f{-1, 0,  0};         break;
            case S_KEY:     dir += math::vec3f{ 0, 0, -1};         break;
            case D_KEY:     dir += math::vec3f{ 1, 0,  0};         break;
            case UP_KEY:    m_editorCameraRot.x -= 0.05; break;
            case LEFT_KEY:  m_editorCameraRot.y -= 0.05; break;
            case DOWN_KEY:  m_editorCameraRot.x += 0.05; break;
            case RIGHT_KEY: m_editorCameraRot.y += 0.05; break;
        }
    }
    m_editorCameraPos += math::mat3x3::rotation(m_editorCameraRot) * dir.normalized() * 0.2;
}

Renderer::Camera EngineIntern::getEditorCamera()
{
    Renderer::Camera cam;

    float zs = 10000.0F / (10000.0F - 0.01F);
    float ys = 1.0F / std::tan((float)(60 * (PI / 180.0F)) * 0.5F);
    float xs = ys; // (ys / aspectRatio)

    cam.projectionMatrix = math::mat4x4(xs,  0,  0,           0,
                                         0, ys,  0,           0,
                                         0,  0, zs, -0.01F * zs,
                                         0,  0,  1,           0);
    cam.viewMatrix = math::mat4x4::translation(m_editorCameraPos) * math::mat4x4::rotation(m_editorCameraRot);
    cam.viewMatrix = cam.viewMatrix.inversed();
    return cam;
}

}