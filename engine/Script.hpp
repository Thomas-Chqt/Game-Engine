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
    Script()              = default;
    Script(const Script&) = default;
    Script(Script&&)      = default;

    void setEntity(Entity);
    
    virtual void onUpdate() = 0;

    virtual ~Script() = default;

protected:
    Entity m_entity;

public:
    Script& operator = (const Script&) = default;
    Script& operator = (Script&&)      = default;
};

}

#endif // SCRIPT_HPP