/*
 * ---------------------------------------------------
 * ScriptLib.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/24 21:09:33
 * ---------------------------------------------------
 */

#include "ScriptLib.hpp"
#include "ECS/Entity.hpp"
#include "Script.hpp"
#include "UtilsCPP/Array.hpp"

extern "C" 
{

PROJECT1_API void getScriptNames(const char*** names, unsigned long* count)
{
    static utils::Array<const char*> nameArray;
    nameArray.clear();
    nameArray.setCapacity(ScriptRegistry::getRegistery().size() + 1);
    for (const auto& [name, _] : ScriptRegistry::getRegistery())
        nameArray.append((const char*)name);
    nameArray.append(nullptr);
    *names = &nameArray[0];
    *count = ScriptRegistry::getRegistery().size();
}

PROJECT1_API GE::Script* makeScriptInstance(const char* name, const GE::Entity& e, GE::Game& g)
{
    return ScriptRegistry::getRegistery()[name](e, g);
}

}