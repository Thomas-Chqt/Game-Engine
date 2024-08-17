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

#include "Game-Engine/Engine.hpp"
#include "Game-Engine/Entity.hpp"
#include "Graphics/Window.hpp"
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
    inline void terminateGame() override { m_running = false; }

    ~EngineIntern() override;

private:
    EngineIntern();

    void scriptSystem();
    Renderer::Camera getActiveCameraSystem();
    void addLightsSystem();
    void addRenderableSystem();

    void onEvent(gfx::Event& event);
    void onImGuiRender();
    void drawSceneGraphWindow();
    void drawEntityInspectorWindow();

    utils::SharedPtr<gfx::Window> m_mainWindow;

    utils::UniquePtr<Game> m_game;

    bool m_running = false;

    // Editor
    Entity m_selectedEntity;

public:
    EngineIntern& operator = (const EngineIntern&) = delete;
    EngineIntern& operator = (EngineIntern&&)      = delete;
};

}

#endif // EngineIntern_hpp