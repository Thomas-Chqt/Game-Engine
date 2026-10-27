#include "Game-Engine/Components.hpp"

#include <cassert>
#include <typeinfo>
#include <mutex>

namespace GE
{

ComponentID componentId(const std::type_info& typeInfo)
{
    static std::mutex mutex;
    static std::map<std::string, ComponentID> componentIDs;
    static ComponentID nextId = 0;

    std::lock_guard lock(mutex);
    auto [it, inserted] = componentIDs.emplace(typeInfo.name(), nextId);
    if (inserted) {
        assert(nextId < maxComponentTypes);
        nextId++;
    }
    return it->second;
}

}
