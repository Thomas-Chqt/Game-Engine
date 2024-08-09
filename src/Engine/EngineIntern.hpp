/*
 * ---------------------------------------------------
 * EngineIntern.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/06/20 16:49:26
 * ---------------------------------------------------
 */

#include "Game-Engine/Game.hpp"
#include "Graphics/Event.hpp"
#include "Graphics/Window.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

class EngineIntern
{
public:
    EngineIntern(const EngineIntern&) = delete;
    EngineIntern(EngineIntern&&)      = delete;

    static inline void init() { s_sharedInstance = utils::UniquePtr<EngineIntern>(new EngineIntern()); }
    static inline EngineIntern& shared() { return *s_sharedInstance; }
    static inline void terminate() { s_sharedInstance.clear(); }

    void runGame(utils::UniquePtr<Game>&&);
    inline void terminateGame() { m_runningGame.clear(); }

    ~EngineIntern();

private:
    EngineIntern();

    void onEvent(gfx::Event& event);

    static inline utils::UniquePtr<EngineIntern> s_sharedInstance;

    utils::UniquePtr<Game> m_runningGame;
    utils::SharedPtr<gfx::Window> m_gameWindow;

public:
    EngineIntern& operator = (const EngineIntern&) = delete;
    EngineIntern& operator = (EngineIntern&&)      = delete;
};

}