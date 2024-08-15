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
    
    static void init(utils::SharedPtr<gfx::Window>&& win);

    inline gfx::Window& mainWindow() override { return *m_mainWindow; }

    void runGame(utils::UniquePtr<Game>&&) override;
    inline void terminateGame() override { m_running = false; }

    inline const utils::Set<int>& pressedKeys() override { return m_pressedKeys; }

    ~EngineIntern() override;

private:
    EngineIntern(utils::SharedPtr<gfx::Window>&&);

    void scriptSystem();
    Renderer::Camera getActiveCameraSystem();
    void addLightsSystem();
    void addRenderableSystem();

    void onEvent(gfx::Event& event);
    void onImGuiRender();
    void drawSceneGraphWindow();
    void drawEntityInspectorWindow();

    utils::SharedPtr<gfx::Window> m_mainWindow;
    Renderer m_renderer;

    utils::UniquePtr<Game> m_game;

    bool m_running = false;

    utils::Set<int> m_pressedKeys;

    // Editor
    Entity m_selectedEntity;

public:
    EngineIntern& operator = (const EngineIntern&) = delete;
    EngineIntern& operator = (EngineIntern&&)      = delete;

};

}

#endif // EngineIntern_hpp