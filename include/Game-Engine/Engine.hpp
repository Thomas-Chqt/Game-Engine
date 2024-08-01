/*
 * ---------------------------------------------------
 * Engine.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/06/20 16:28:28
 * ---------------------------------------------------
 */

#ifndef ENGINE_HPP
# define ENGINE_HPP

#include "Game-Engine/Game.hpp"
#include "Game-Engine/Mesh.hpp"
#include "UtilsCPP/Array.hpp"
#include "UtilsCPP/String.hpp"
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

class Engine
{
public:
    Engine(const Engine&) = delete;
    Engine(Engine&&)      = delete;

    static void init();
    inline static Engine& shared() { return *s_sharedInstance; }
    inline static void terminate() { s_sharedInstance.clear(); }

    virtual void runGame(utils::UniquePtr<Game>&&) = 0;
    virtual void terminateGame() = 0;

    virtual void showEditorUI() = 0;
    virtual void hideEditorUI() = 0;

    virtual utils::Array<Mesh> loadMeshes(const utils::String& filePath) = 0;
    
    virtual ~Engine() = default;

protected:
    Engine() = default;

    inline static utils::UniquePtr<Engine> s_sharedInstance;

public:
    Engine& operator = (const Engine&) = delete;
    Engine& operator = (Engine&&)      = delete;

};

}

#endif // ENGINE_HPP