/*
 * ---------------------------------------------------
 * EngineIntern.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/15 15:12:11
 * ---------------------------------------------------
 */

#ifndef EngineIntern_hpp
#define EngineIntern_hpp

#include "Engine/EditorCamera.hpp"
#include "Game-Engine/Engine.hpp"
#include "Game-Engine/Entity.hpp"
#include "Game-Engine/InputContext.hpp"
#include "Graphics/FrameBuffer.hpp"
#include "Graphics/Window.hpp"
#include "Math/Vector.hpp"
#include "Renderer/Renderer.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

class EngineIntern : public Engine
{
public:
    EngineIntern(const EngineIntern&) = delete;
    EngineIntern(EngineIntern&&)      = delete;
    
    static inline void init() { Engine::s_sharedInstance = utils::UniquePtr<EngineIntern>(new EngineIntern).staticCast<Engine>(); }

    inline gfx::Window& mainWindow() override { return *m_mainWindow; }

    void runGame(utils::UniquePtr<Game>&&) override;
    inline void terminateGame() override { m_gameRunning = false; }

    void editorForGame(utils::UniquePtr<Game>&&) override;

    ~EngineIntern() override;

private:
    EngineIntern();

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

    utils::SharedPtr<gfx::Window> m_mainWindow;

    utils::UniquePtr<Game> m_game;

    bool m_gameRunning = false;
    bool m_editorRunning = false;

    // Editor
    InputContext m_editorInputContext;
    EditorCamera m_editorCamera;
    Entity m_selectedEntity;

    utils::SharedPtr<gfx::FrameBuffer> m_viewportFBuff;
    math::vec2f m_viewportPanelSize = {800, 600};
    bool m_viewportPanelSizeIsDirty = true;

public:
    EngineIntern& operator = (const EngineIntern&) = delete;
    EngineIntern& operator = (EngineIntern&&)      = delete;
};

}

#endif // EngineIntern_hpp