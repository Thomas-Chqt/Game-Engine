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
#include "Script.hpp"

namespace GE
{

class Game
{
public:
    struct Descriptor
    {
        utils::Set<Scene> scenes;
        InputContext inputContext;

        gfx::GraphicAPI* graphicAPI = nullptr;
        std::filesystem::path baseDir;
        MakeScriptInstanceFn makeScriptInstance = nullptr;
        utils::Func<void()> stopFunc;
    };

public:
    Game()            = default;
    Game(const Game&) = default;
    Game(Game&&)      = default;

    Game(const Descriptor&);

    const utils::Func<void()> stop;

    Scene& activeScene();
    inline const Scene& activeScene() const { return const_cast<Game*>(this)->activeScene(); }

    void setActiveScene(const utils::String& name);

    inline InputContext& inputContext() { return m_inputContext; }
    inline const InputContext& inputContext() const { return m_inputContext; }

    ~Game() = default;

private:
    utils::Set<Scene> m_scenes;
    Scene* m_activeScene = nullptr;
    InputContext m_inputContext;

    gfx::GraphicAPI* m_graphicAPI = nullptr;
    std::filesystem::path m_baseDir;
    MakeScriptInstanceFn m_makeScriptInstance = nullptr;

public:
    Game& operator = (const Game&) = delete;
    Game& operator = (Game&&)      = delete;
};

}

#endif // GAME_HPP