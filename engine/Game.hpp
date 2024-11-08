/*
 * ---------------------------------------------------
 * Game.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/11/07 13:16:05
 * ---------------------------------------------------
 */

#ifndef GAME_HPP
#define GAME_HPP

#include "InputManager/InputContext.hpp"
#include "Scene.hpp"
#include "UtilsCPP/Set.hpp"
#include "UtilsCPP/String.hpp"

namespace GE
{

class Game
{
public:
    Game()            = default;
    Game(const Game&) = default;
    Game(Game&&)      = default;

    Game(const utils::Set<Scene>&);

    void start(gfx::GraphicAPI& api, const std::filesystem::path& baseDir, MakeScriptInstanceFn);
    void stop();
    inline bool isRunning() const { return m_isRunning; }

    inline Scene& activeScene() { return *m_activeScene; }
    inline const Scene& activeScene() const { return *m_activeScene; }

    void setActiveScene(const utils::String& name);

    inline InputContext& inputContext() { return m_inputContext; }
    inline const InputContext& inputContext() const { return m_inputContext; }

    ~Game() = default;

private:
    utils::Set<Scene> m_scenes;
    Scene* m_activeScene = nullptr;

    gfx::GraphicAPI* m_api = nullptr;
    std::filesystem::path m_baseDir;
    MakeScriptInstanceFn m_makeScriptInstance = nullptr;
    InputContext m_inputContext;

    bool m_isRunning = false;

public:
    Game& operator = (const Game&) = default;
    Game& operator = (Game&&)      = default;
};

}

#endif // GAME_HPP