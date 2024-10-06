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

#include "UtilsCPP/Set.hpp"
#include "UtilsCPP/String.hpp"
#include "Scene.hpp"

namespace GE
{

class Game
{
public:
    Game()            = delete;
    Game(const Game&) = delete;
    Game(Game&&)      = delete;
    
    Game(utils::Set<Scene>& scenes);

    virtual inline void onSetup() {};
    virtual inline void onUpdate() {};

    inline void addScene(const utils::String& name, Scene&& scene) { m_scenes.insert(std::move(scene)); };
    void deleteScene(const utils::String&);

    void setStartScene(const utils::String& name);


    virtual ~Game() = default;

private:
    utils::Set<Scene>& m_scenes;
    Scene* m_startScene;
    
public:
    Game& operator = (const Game&) = delete;
    Game& operator = (Game&&)      = delete;
};

}

#endif // GAME_HPP