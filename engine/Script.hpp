/*
 * ---------------------------------------------------
 * Script.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/24 12:39:19
 * ---------------------------------------------------
 */

#ifndef SCRIPT_HPP
#define SCRIPT_HPP

#include "ECS/Entity.hpp"

namespace GE
{

class Game;
class Script;

using GetScriptNamesFn = void (*)(const char***, unsigned long*); // getScriptNames
using MakeScriptInstanceFn = Script* (*)(const char*, const Entity&, Game& g); // makeScriptInstance

class Script
{
public:
    Script()              = delete;
    Script(const Script&) = default;
    Script(Script&&)      = default;

    inline Script(const Entity& e, Game& g) : m_entity(e), m_game(g) {}
    
    virtual void onUpdate() {};

    virtual ~Script() = default;

protected:
    Entity m_entity;
    Game& m_game;
};

}

#endif // SCRIPT_HPP