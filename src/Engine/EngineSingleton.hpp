/*
 * ---------------------------------------------------
 * EngineSingleton.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/06/20 16:49:26
 * ---------------------------------------------------
 */

#include "Game-Engine/Game.hpp"
#include "Graphics/Event.hpp"
#include "Graphics/Window.hpp"
#include "Renderer/Renderer.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

class EngineSingleton
{
public:
    EngineSingleton(const EngineSingleton&) = delete;
    EngineSingleton(EngineSingleton&&)      = delete;

    static inline void init() { s_sharedInstance = utils::UniquePtr<EngineSingleton>(new EngineSingleton()); }
    static inline EngineSingleton& shared() { return *s_sharedInstance; }
    static inline void terminate() { s_sharedInstance.clear(); }

    void onEvent(gfx::Event& event);

    void runGame(utils::UniquePtr<Game>&&);
    inline void terminateGame() { m_running = false; }
    
    ~EngineSingleton() = default;

private:
    EngineSingleton() = default;

    static utils::UniquePtr<EngineSingleton> s_sharedInstance;

    utils::UniquePtr<Game> m_game;
    bool m_running = false;

    utils::SharedPtr<gfx::Window> m_gameWindow;
    float m_aspectRatio = 800.0f / 600.0f;

    Renderer m_gameRenderer;

public:
    EngineSingleton& operator = (const EngineSingleton&) = delete;
    EngineSingleton& operator = (EngineSingleton&&)      = delete;

};

}