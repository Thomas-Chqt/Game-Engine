/*
 * ---------------------------------------------------
 * InternalEngine.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/30 16:48:53
 * ---------------------------------------------------
 */

#ifndef INTERNALENGINE_HPP
#define INTERNALENGINE_HPP

#include "Game-Engine/ECSWorld.hpp"
#include "Game-Engine/Engine.hpp"
#include "Graphics/Event.hpp"
#include "Graphics/GraphicAPI.hpp"
#include "Graphics/Window.hpp"
#include "Renderer/Renderer.hpp"
#include "UtilsCPP/Set.hpp"
#include "UtilsCPP/SharedPtr.hpp"

namespace GE
{

class InternalEngine : public Engine
{
public:
    InternalEngine();
    InternalEngine(const InternalEngine&) = delete;
    InternalEngine(InternalEngine&&)      = delete;
   
    void runGame(utils::UniquePtr<Game>&&) override;
    inline void terminateGame() override { m_running = false; }

    inline void showEditorUI() override { m_renderer.setUI(utils::Func<void ()>(*this, &InternalEngine::makeUI)); };
    inline void hideEditorUI() override { m_renderer.setUI(utils::Func<void ()>()); };

    utils::Array<Mesh> loadMeshes(const utils::String& filePath) override;


    ~InternalEngine() override;

private:
    void onEvent(gfx::Event& event);
    void makeUI();

    utils::SharedPtr<gfx::Window> m_window;
    utils::SharedPtr<gfx::GraphicAPI> m_graphicAPI;
    
    Renderer m_renderer;

    utils::UniquePtr<Game> m_runningGame;
    bool m_running = false;

    utils::Set<int> m_pressedKeys;

    ECSWorld::Entity m_selectedEntity;

public:
    InternalEngine& operator = (const InternalEngine&) = delete;
    InternalEngine& operator = (InternalEngine&&)      = delete;
};

}

#endif // INTERNALENGINE_HPP