/*
 * ---------------------------------------------------
 * InternalComponents.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/07 16:40:47
 * ---------------------------------------------------
 */

#ifndef INTERNALCOMPONENTS_HPP
#define INTERNALCOMPONENTS_HPP

#include "Game-Engine/Entity.hpp"
#include "UtilsCPP/String.hpp"
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

struct NameComponent
{
    utils::String name;

    inline operator utils::String& () { return name; }
};

struct ScriptComponent
{
    utils::UniquePtr<Entity> instance;
};

struct HierarchicalComponent
{
    Entity parent;
    Entity firstChild;
    Entity nextChild;
};

}

#endif // INTERNALCOMPONENTS_HPP