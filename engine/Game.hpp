/*
 * ---------------------------------------------------
 * Game.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/08 12:32:48
 * ---------------------------------------------------
 */

#ifndef GAME_HPP
#define GAME_HPP

#include "UtilsCPP/String.hpp"
#include "UtilsCPP/Dictionary.hpp"
#include "Scene.hpp"

namespace GE
{

class Game
{
public:
    Game()            = default;
    Game(const Game&) = delete;
    Game(Game&&)      = delete;
    
    inline void addScene(const utils::String& name, Scene&& scene) { m_scenes.insert(name, std::move(scene)); };
    inline Scene& getScene(const utils::String& name) { return m_scenes[name]; }
    void deleteScene(const utils::String&);
    inline const utils::Dictionary<utils::String, Scene>& scenes() { return m_scenes; }

    inline const utils::String& startSceneName() { return m_startSceneName; }
    void setStartScene(const utils::String& name);

    virtual inline void onSetup() {};
    virtual inline void onUpdate() {};

    virtual ~Game() = default;

private:
    utils::Dictionary<utils::String, Scene> m_scenes;
    utils::String m_startSceneName;
    
public:
    Game& operator = (const Game&) = delete;
    Game& operator = (Game&&)      = delete;
};

}

#endif // GAME_HPP