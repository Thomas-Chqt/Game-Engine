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

class Script
{
public:
    Script()              = delete;
    Script(const Script&) = default;
    Script(Script&&)      = default;

    inline Script(const Entity& e) : m_entity(e) {}
    
    virtual void onUpdate() {};

    virtual ~Script() = default;

protected:
    Entity m_entity;

public:
    Script& operator = (const Script&) = default;
    Script& operator = (Script&&)      = default;
};

using GetScriptNamesFn = void (*)(const char***, unsigned long*); // getScriptNames
using MakeScriptInstanceFn = Script* (*)(const char*, const Entity&); // makeScriptInstance

}

#endif // SCRIPT_HPP