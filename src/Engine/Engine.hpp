/*
 * ---------------------------------------------------
 * Engine.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/31 12:46:07
 * ---------------------------------------------------
 */

#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "Engine/EditorCamera.hpp"
#include "Game-Engine/Entity.hpp"
#include "Game-Engine/Game.hpp"
#include "Game-Engine/InputContext.hpp"
#include "Graphics/FrameBuffer.hpp"
#include "Graphics/Window.hpp"
#include "Renderer/Renderer.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/String.hpp"
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

class Engine
{
public:
    Engine(const Engine&) = delete;
    Engine(Engine&&)      = delete;
    
    inline static void init() { s_sharedInstance = utils::UniquePtr<Engine>(new Engine()); }
    inline static Engine& shared() { return *s_sharedInstance; }
    inline static void terminate() { s_sharedInstance.clear(); }

    void openProject(const utils::String& filepath);

    void run();

    ~Engine();

private:
    Engine();

    void onEvent(gfx::Event& event);
    void onImGuiRender();

    void updateVPFrameBuff();

    // ImGuiPanels
    void drawViewportPanel();
    void drawSceneGraphPanel();
    void drawEntityInspectorPanel();
    void drawFPSPanel();

    // Systems
    void scriptSystem();
    Renderer::Camera getActiveCameraSystem();
    void addLightsSystem();
    void addRenderableSystem();

    static utils::UniquePtr<Engine> s_sharedInstance;

    utils::SharedPtr<gfx::Window> m_window;
    Renderer m_renderer;
    bool m_editorRunning = false;

    utils::UniquePtr<Game> m_game;
    bool m_gameRunning = false;

    // Editor
    InputContext m_editorInputContext;
    EditorCamera m_editorCamera;
    Entity m_selectedEntity;

    utils::SharedPtr<gfx::FrameBuffer> m_viewportFBuff;
    math::vec2f m_viewportPanelSize = {800, 600};
    bool m_viewportPanelSizeIsDirty = true;

public:
    Engine& operator = (const Engine&) = delete;
    Engine& operator = (Engine&&)      = delete;
};

}

#endif // ENGINE_HPP