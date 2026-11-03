#pragma once

#include "Game-Engine/Components.hpp"

#include <cassert>

namespace GE
{

template<Component T>
struct ComponentCloneTraits
{
    static T clone(const T& source, const EntityIDMap&) { return T(source); }
};

template<>
struct ComponentCloneTraits<EntityID>
{
    static EntityID clone(const EntityID& source, const EntityIDMap& ids)
    {
        const auto it = ids.find(source);
        assert(it != ids.end());
        return it->second;
    }
};

template<>
struct ComponentCloneTraits<ParentComponent>
{
    static ParentComponent clone(const ParentComponent& source, const EntityIDMap& ids)
    {
        return {
            .parent=source.parent.transform([&](EntityID id) { return ComponentCloneTraits<EntityID>::clone(id, ids); }),
            .nextChild=source.nextChild.transform([&](EntityID id) { return ComponentCloneTraits<EntityID>::clone(id, ids); })
        };
    }
};

template<>
struct ComponentCloneTraits<ChildrenComponent>
{
    static ChildrenComponent clone(const ChildrenComponent& source, const EntityIDMap& ids)
    {
        return {source.firstChild.transform([&](EntityID id) { return ComponentCloneTraits<EntityID>::clone(id, ids); })};
    }
};

template<>
struct ComponentCloneTraits<ScriptComponent>
{
    static ScriptComponent clone(const ScriptComponent& source, const EntityIDMap&)
    {
        return {
            .name = source.name,
            .parameters = source.parameters,
            .instance = {}
        };
    }
};

}
