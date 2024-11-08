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
    
    Game(const utils::Set<Scene>& scene, const utils::String& startScene);

    void start(gfx::GraphicAPI& api, const std::filesystem::path& baseDir, Script *(*makeScriptInstanceFn)(char *));
    void stop();
    inline bool isRunning() const { return m_isRunning; }

    inline Scene& activeScene() const { return *m_activeScene; }
    void setActiveScene(const utils::String& name);

    ~Game() = default;

private:
    utils::Set<Scene> m_scenes;
    Scene* m_activeScene = nullptr;

    gfx::GraphicAPI* m_api = nullptr;
    std::filesystem::path m_baseDir;
    Script *(*m_makeScriptInstanceFn)(char *) = nullptr;

    bool m_isRunning = false;

public:
    Game& operator = (const Game&) = default;
    Game& operator = (Game&&)      = default;
};

}

#endif // GAME_HPP